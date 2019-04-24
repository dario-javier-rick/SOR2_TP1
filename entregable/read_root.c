#include <stdio.h>
#include <stdlib.h>
#include "fatstructs.h"

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
     case 0x20:
       printf("Es un archivo \n");
       break;
     default:
       break;
    }

	//printf("Fecha creacion: %s \n", entry->createdTime);
	//printf("Hora creacion: %s \n", entry->createdHour);
	printf("Cluster inicio: %d \n", entry->cluster_inicio);
	printf("Tamanio archivo: %s \n", entry->tamanio_archivo);

}

void mostrarContenidoArchivo(FILE* in, Fat12Entry* entry, int tamanioEntry, Fat12BootSector* bs)
{
   unsigned int inicioFat = sizeof(Fat12BootSector) + (bs->sectores_reservados - 1) * bs->sector_size;
   unsigned int inicioRoot = inicioFat + bs->tamanio_fat * bs->cantidad_tablas_fats * bs->sector_size;
   unsigned int inicioData = inicioRoot + (bs->root_entries * tamanioEntry); //offset

   printf("Inicio FAT: [0x%X] \n", inicioFat);
   printf("Inicio Root: [0x%X] \n", inicioRoot);
  // printf("SizeOf entry: %d \n", sizeof(entry));
  // printf("Tamanio entry: %d \n", tamanioEntry);
   printf("Inicio Data: [0x%X] \n", inicioData);
   //printf("Sectores por track: [0x%X] \n", bs->sectores_por_track);
   //printf("Tamaño de sector: [0x%X] \n", bs->sector_size);
   //printf("Cluster inicio: [0x%X] \n", entry->cluster_inicio);

   unsigned char buffer[bs->sector_size]; //Tomo este tamaño porque voy a leer sector por sector
   unsigned int tamanioCluster = bs->sectores_por_cluster * bs->sector_size;
   unsigned int bloque = inicioData + (tamanioCluster * (entry->cluster_inicio - 2) ); //Calculo bloque de inicio del dato
   fseek(in, bloque, SEEK_SET); //Voy al bloque de inicio
   fread(buffer, 1, bs->sector_size, in); //Leo el sector

   printf("Bloque [0x%X] :", bloque);
   printf("%s \n", buffer);

   //printf("\nAhora en 0x%lX\n", ftell(in));
   //Voy a la tabla FAT a buscar el proximo bloque, etc...
   //Lo correcto sería hacer un while hasta que los bytes vengan vacíos, o hasta que llegue al limite del bloque
}

void print_file_info(FILE* in, Fat12Entry *entry, int tamanioEntry, Fat12BootSector *bs, int posicion) 
{

   unsigned int inicioFat = sizeof(Fat12BootSector) + (bs->sectores_reservados - 1) * bs->sector_size;
   unsigned int inicioRoot = inicioFat + bs->tamanio_fat * bs->cantidad_tablas_fats * bs->sector_size;
   unsigned int inicioData = inicioRoot + (bs->root_entries * tamanioEntry); //offset

   //printf("Inicio Data: [0x%X] \n", inicioData);

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
                printf("Cluster de inicio [0x%lX] \n", entry->cluster_inicio + inicioData); //tengo que sumar offset de first allocation unit
                //First allocation unit empieza en 0x5800
                printf("Tamaño de archivo [%i] bytes \n", (int)entry->tamanio_archivo[0]);
	}

       mostrarAtributos(entry);
       mostrarContenidoArchivo(in, entry, tamanioEntry, bs);
}

void leerDirectorio(FILE* in, Fat12Entry entry, int ultimoSectorLeido, int cantidadEntradas, Fat12BootSector bs)
{
	int i = 0;
	for (i = 0; i < cantidadEntradas; i++) {
		//printf("\nAhora en 0x%lX\n", ftell(in));
		fread(&entry, sizeof(entry), 1, in);
		ultimoSectorLeido = ftell(in);
		print_file_info(in, &entry, sizeof(entry), &bs, i);
		if(entry.atributos == 0x10 && entry.filename[0] != 0x2E) //Si estoy en un directorio y no estoy leyendo el puntero al anterior (evito loop infinito)
		{
			printf("\nEntrando en subdirectorio %s\n", entry.filename);

			unsigned int inicioDataDirectorio = (/*offset inicioData*/0x4A00 + (bs.sectores_por_cluster * bs.sector_size * (entry.cluster_inicio - 2)));
			printf("\ninicioDataDirectorio 0x%lX\n", inicioDataDirectorio);
			fseek(in, inicioDataDirectorio, SEEK_SET);

			//Recursion ATR			
			leerDirectorio(in, entry, ftell(in), bs.sectores_por_cluster, bs);

			printf("\nSaliendo del subdirectorio %s\n", entry.filename);
			//Volver al ultimoSectorLeido antes de entrar al directorio
		}
		fseek(in, ultimoSectorLeido, SEEK_SET);
	}
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

	leerDirectorio(in, entry, ftell(in), bs.root_entries, bs);

        printf("\n----------\n");
	printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
	fclose(in);
	return 0;
}
