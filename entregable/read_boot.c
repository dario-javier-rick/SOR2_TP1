#include <stdio.h>
#include <stdlib.h>

typedef struct {
	unsigned char first_byte;
	unsigned char start_chs[3];
	unsigned char partition_type;
	unsigned char end_chs[3];
	char start_sector[4];
	char length_sectors[4];
} __attribute((packed)) PartitionTable;

typedef struct {
    //Bytes 0-35
    unsigned char jmp[3]; //0-2
    char oem[8]; //3-10
    unsigned short sector_size; //11-12 2 bytes
    unsigned char sectores_por_cluster; //13 1 byte
    unsigned short sectores_reservados; //14-15 2 bytes
    unsigned char cantidad_tablas_fats; //16 1 byte
    unsigned short root_entries; //17-18 2 bytes
    unsigned short cantidad_sectores; //19-20 2 bytes
    unsigned char media_descriptor; //21 1 byte
    unsigned short tamanio_fat; //23-23 2 bytes , en sectores
    unsigned short sectores_por_track; //24-25 2 bytes
    unsigned short cantidad_heads; //26-27 2 bytes
    long sectores_ocultos; //28-31 4 bytes
    long sectores_filesystem; //32-35 4 bytes
	
	//Bytes restantes
	//unsigned char int13; //36	BIOS INT 13h (low level disk services) drive number
	//unsigned char nu; //37	Not used
	//unsigned char ebs; //38	Extended boot signature to validate next three fields (0x29)
    char volume_id[4]; //39-42
    char volume_label[11]; //43-53
    char fs_type[8]; //54-61 en ascii
    char boot_code[448]; //62-509
    unsigned short boot_sector_signature; //510-511
} __attribute((packed)) Fat12BootSector;

int main() {
	FILE * in = fopen("test.img", "rb");
	int i;
	PartitionTable pt[4];
	Fat12BootSector bs;

	fseek(in, 0x1BE, SEEK_SET); // Ir al inicio de la tabla de particiones. La misma se encuentra en el MBR a partir del byte 446 (1BE)	
	fread(pt, sizeof(PartitionTable), 4, in); // leo entradas 

	for (i = 0; i < 4; i++) {
		printf("Partition type: %d\n", pt[i].partition_type);
		if (pt[i].partition_type == 1) {
			printf("Encontrado FAT12 %d\n", i);
			break;
		}
	}

	if (i == 4) {
		printf("No FAT12 filesystem found, exiting...\n");
		return -1;
	}

	fseek(in, 0, SEEK_SET);
	fread(&bs, sizeof(Fat12BootSector), 1, in);

    printf("  Jump code: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);
    printf("  OEM Code: [%.8""s]\n", bs.oem);
    printf("  sector_size: %d\n", bs.sector_size);
    printf("  sectores por cluster: %d\n", bs.sectores_por_cluster);
    printf("  sectores reservados: %d\n", bs.sectores_reservados);
    printf("  numero de tablas fat: %d\n ", bs.cantidad_tablas_fats);
    printf("  root entries: %d\n ", bs.root_entries);
    printf("  cantidad_sectores: %d\n ", bs.cantidad_sectores);
    printf("  media_descriptor: %d\n ", bs.media_descriptor);
    printf("  tamanio fat: %d\n ", bs.tamanio_fat);
    printf("  sectores por track: %d\n ", bs.sectores_por_track);
    printf("  cantidad heads: %d\n ", bs.cantidad_heads);
    printf("  sectores ocultos: %d\n ", bs.sectores_ocultos);
    printf("  sectores filesystem: %d\n ", bs.sectores_filesystem);


    printf("  Volume label: [%.11""s]\n", bs.volume_label);
    printf("  Filesystem type: [%.8s]\n", bs.fs_type);
    printf("  Boot sector signature: 0x%04X\n", bs.boot_sector_signature);

	fclose(in);
	return 0;
}
