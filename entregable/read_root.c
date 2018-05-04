#include <stdio.h>
#include <stdlib.h>

typedef struct {
	unsigned char first_byte;
	unsigned char start_chs[3];
	unsigned char partition_type;
	unsigned char end_chs[3];
	unsigned short starting_cluster;
	unsigned int file_size;
} __attribute((packed)) PartitionTable;

typedef struct {
    //Bytes 0-35
    unsigned char jmp[3]; //0-2
    unsigned char oem[8]; //3-10
    unsigned short sector_size; //11-12 2 bytes
    unsigned char sectores_por_cluster; //13 1 byte
    unsigned short sectores_reservados; //14-15 2 bytes
    unsigned char cantidad_tablas_fats; //16 1 byte
    unsigned short root_entries; //17-18 2 bytes
    unsigned short cantidad_sectores; //19-20 2 bytes
    unsigned char media_descriptor[1]; //21 1 byte
    unsigned short tamanio_fat; //23-23 2 bytes (en sectores)
    unsigned short sectores_por_track; //24-25 2 bytes
    unsigned short cantidad_heads; //26-27 2 bytes
    unsigned int sectores_ocultos:32; //28-31 4 bytes
    unsigned int sectores_filesystem:32; //32-35 4 bytes

    //Bytes restantes
    unsigned char int13; //36	BIOS INT 13h (low level disk services) drive number
    unsigned char nu; //37	Not used
    unsigned char ebs; //38	Extended boot signature to validate next three fields (0x29)
    char volume_id[4]; //39-42
    char volume_label[11]; //43-53
    char fs_type[8]; //54-61 en ascii
    char boot_code[448]; //62-509
    unsigned short boot_sector_signature; //510-511
} __attribute((packed)) Fat12BootSector;

typedef struct {
/*
	unsigned char filename[8];
	unsigned char ext[3];
	unsigned char atributos[1];
	unsigned char reservado[10];
	unsigned short hora_modificacion;
	unsigned short fecha_modificacion;
	unsigned short cluster_inicio;
	unsigned char tamanio_archivo[4];
*/
	unsigned char filename[8];
	unsigned char ext[3];
	unsigned char atributos;
	unsigned char reservado;
	unsigned char createdTime;
	unsigned char createdHour[2];
	unsigned char createdDay[2];
	unsigned char accessedDay[2];
	unsigned char highBytesOfFirstClusterAddress[2];
	unsigned char writenTime[2];
	unsigned char writenDay[2];
	unsigned short cluster_inicio;
	unsigned char tamanio_archivo[4];

} __attribute((packed)) Fat12Entry;

void mostrarAtributos(Fat12Entry *entry)
{
   switch (entry->atributos) {
     case 0x01:
       printf("Es un archivo de solo lectura \n");
       break;
     case 0x02:
       printf("Es un archivo oculto \n");
       break;
     case 0x04:
       printf("Es un archivo de sistema \n");
       break;
     case 0x10:
       printf("Es un subdirectorio \n");
       break;
     default:
       break;
    }
}

void mostrarContenidoArchivo(Fat12Entry* entry, Fat12BootSector* bs)
{
   unsigned int inicioFat = sizeof(Fat12BootSector) + (bs->sectores_reservados - 1) * bs->sector_size;
   unsigned int inicioRoot = inicioFat + bs->tamanio_fat * bs->cantidad_tablas_fats * bs->sector_size;
   unsigned int inicioData = inicioRoot + (bs->root_entries * sizeof(entry)); //offset

   printf("Inicio FAT: [0x%X] \n", inicioFat);
   printf("Inicio Root: [0x%X] \n", inicioRoot);
   printf("SizeOf entry: [0x%lX] \n", sizeof(entry));
   printf("Inicio Data: [0x%X] \n", inicioData);
}

void print_file_info(Fat12Entry *entry, Fat12BootSector *bs, int posicion) {

        printf("sizeof(entry) [pfi]: [0x%lX] \n", sizeof(entry));

	switch (entry->filename[0]) {
	case 0x00:
		return; // unused entry
	case 0xE5:
                printf("\n----------\n");
	        printf("Deleted file: [?%.7s.%.3s] ", entry->filename + 1, entry->ext);
		break;
	case 0x05:
                printf("\n----------\n");
		printf("File starting with 0xE5: [%c%.7s.%.3s] ", 0xE5, entry->filename + 1, entry->ext);
                break;
	case 0x2E:
                printf("\n----------\n");
		printf("Directory: [%.8s.%.3s] ", entry->filename, entry->ext);
		break;
	default: //Si cae en este caso, es el primer caracter del archivo
                printf("\n----------\n");
                printf("File: [%.8s.%.3s] \n", entry->filename, entry->ext);
                printf("Cluster de inicio [0x%X] \n", entry->cluster_inicio); //tengo que sumar offset de first allocation unit
                //First allocation unit empieza en 0x5800
                printf("Tamaño de archivo [%i] bytes \n", (int)entry->tamanio_archivo[0]);
	}

       mostrarAtributos(entry);
       mostrarContenidoArchivo(entry,bs);
}

int main() {
	FILE * in = fopen("test.img", "rb");
	int i;
	PartitionTable pt[4];
	Fat12BootSector bs;
	Fat12Entry entry;

	fseek(in, 0x1BE, SEEK_SET); //Ir al inicio de la tabla de particiones
	fread(pt, sizeof(PartitionTable), 4, in); //Lectura

	for (i = 0; i < 4; i++) {
		if (pt[i].partition_type == 1) {
			printf("Encontrada particion FAT12 %d\n", i);
			break;
		}
	}

	if (i == 4) {
		printf("No encontrado filesystem FAT12, saliendo...\n");
		return -1;
	}

	fseek(in, 0, SEEK_SET);
	fread(&bs, sizeof(Fat12BootSector), 1, in);// Leo boot sector

	printf("En  0x%lx, sector size %d, FAT size %d sectors, %d FATs\n\n",
		ftell(in), bs.sector_size, bs.tamanio_fat, bs.cantidad_tablas_fats);


        //Voy al final de la ultima tabla FAT, donde comienza el root directory
	fseek(in, (bs.sectores_reservados - 1 + bs.tamanio_fat * bs.cantidad_tablas_fats) *
		bs.sector_size, SEEK_CUR);
        printf("Sectores reservados: %i\n", bs.sectores_reservados);
        printf("Tamaño de sector: %i\n", bs.sector_size);
        printf("Cantidad tablas FAT: %i\n", bs.cantidad_tablas_fats);
        printf("Tamaño de FAT: %i\n", bs.tamanio_fat);
        printf("\nInicio Root directory en 0x%lX\n", ftell(in));


	printf("Root dir_entries %d \n", bs.root_entries);
	for (i = 0; i < bs.root_entries; i++) {
                //printf("\nAhora en 0x%lX\n", ftell(in));
		fread(&entry, sizeof(entry), 1, in);
		printf("sizeof(entry): [0x%lX] \n", sizeof(entry));
		print_file_info(&entry, &bs, i);
	}

        printf("\n----------\n");
	printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
	fclose(in);
	return 0;
}
