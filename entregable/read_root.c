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
	unsigned char jmp[3];
	char oem[8];
	unsigned short sector_size; // 2 bytes
	//Dario Rick - ini
	char sectores_por_cluster; // 1 byte
	unsigned short sectores_reservados; // 2 bytes
	char cantidad_tablas_fats; // 1 byte
	short root_entries; // 2 bytes
	short cantidad_sectores; // 2 bytes
	char media_descriptor; // 1 byte
	short sectores_por_fat; // 2 bytes
	short sectores_por_head; // 2 bytes
	short heads_por_cilindro; // 2 bytes
	long sectores_ocultos; // 4 bytes
	long big_number_sectores; // 4 bytes
	long big_sectores_por_fat; // 4 bytes
	short extFlags; // 2 bytes
	short FSVersion; // 2 bytes
	long directorio_inicio; // 4 bytes
	short fs_info_sector; // 2 bytes
	short backup_boot_sector; // 2 bytes
	//Dario Rick - fin
	char volume_id[4];
	char volume_label[11];
	char fs_type[8]; // Type in ascii
	char boot_code[448];
	unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct {
	//Dario Rick - INI
	unsigned char filename[8];
	unsigned char ext[3];
	unsigned char atributos;
	unsigned char reservado[10];
	unsigned short hora_modificacion;
	unsigned short fecha_modificacion;
	unsigned short cluster_inicio;
	unsigned long tamanio_archivo;
	//Dario Rick - FIN
} __attribute((packed)) Fat12Entry;

void print_file_info(Fat12Entry *entry) {
	switch (entry->filename[0]) {
	case 0x00:
		return; // unused entry
	case 0xE5:
		printf("Deleted file: [?%.7s.%.3s]\n", entry->filename + 1, entry->ext);
		return;
	case 0x05:
		printf("File starting with 0xE5: [%c%.7s.%.3s]\n", 0xE5, entry->filename + 1, entry->ext);
		break;
	case 0x2E:
		printf("Directory: [%.8s.%.3s]\n", entry->filename, entry->ext);
		break;
	default:
		printf("File: [%.8s.%.3s]\n", entry->filename, entry->ext);
	}

}

int main() {
	FILE * in = fopen("test.img", "rb");
	int i;
	PartitionTable pt[4];
	Fat12BootSector bs;
	Fat12Entry entry;

	printf("tamaño PartitionTable: %i\n", sizeof(PartitionTable));
	//Dario Rick - INI
	fseek(in, 0x1BE, SEEK_SET); //Ir al inicio de la tabla de particiones
	fread(pt, sizeof(PartitionTable), 4, in); //Lectura
	//Dario Rick - FIN

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

	printf("tamaño fat12BootSector: %i\n", sizeof(Fat12BootSector));

	fread(&bs, /*sizeof(Fat12BootSector)*/0, 1, in);// Leo boot sector

	printf("En  0x%X, sector size %d, FAT size %d sectors, %d FATs\n\n",
		ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);

	fseek(in, (bs.reserved_sectors - 1 + bs.fat_size_sectors * bs.number_of_fats) *
		bs.sector_size, SEEK_CUR);

	printf("Root dir_entries %d \n", bs.root_dir_entries);
	for (i = 0; i < bs.root_dir_entries; i++) {
		fread(&entry, sizeof(entry), 1, in);
		print_file_info(&entry);
	}

	printf("\nLeido Root directory, ahora en 0x%X\n", ftell(in));
	fclose(in);
	return 0;
}
