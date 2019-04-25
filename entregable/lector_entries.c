#include <stdio.h>
#include <stdlib.h>
#include "fatstructs.h"

//Punto 3a
void print_file_name(Fat12Entry *entry) 
{
	if (entry->atributos == 0x10 || entry->atributos == 0x20)
	{
		switch (entry->filename[0])
		{
			case 0xE5:
				printf("\nArchivo eliminado. ");
				break;
			case 0x2E:
				printf("\nPuntero al directorio actual o al anterior.\n");
				return;
			default:
				break;
		}
		printf("\nFile: [%.8s.%.3s] \n", entry->filename, entry->ext);
	}
}

void leerDirectorio(FILE* in, Fat12Entry entry, int ultimoSectorLeido, int cantidadEntradas, Fat12BootSector bs)
{
	printf("\nLeyendo directorio\n");
	int i = 0;
	for (i = 0; i < cantidadEntradas; i++) {
		//printf("\nAhora en 0x%lX\n", ftell(in));
		fread(&entry, sizeof(entry), 1, in);
		ultimoSectorLeido = ftell(in);
		//print_file_info(in, &entry, sizeof(entry), &bs, i);
		print_file_name(&entry);
		if(entry.atributos == 0x10 && entry.filename[0] != 0x2E) //Si estoy en un directorio y no estoy leyendo el puntero al anterior (evito loop infinito)
		{
			printf("\nEntrando en subdirectorio %s\n", entry.filename);
			unsigned int inicioDataDirectorio = (/*offset inicioData*/0x4A00 + (bs.sectores_por_cluster * bs.sector_size * (entry.cluster_inicio - 2)));
			printf("\ninicioDataDirectorio 0x%lX\n", inicioDataDirectorio);
			fseek(in, inicioDataDirectorio, SEEK_SET);
			//Recursion ATR			
			leerDirectorio(in, entry, ftell(in), bs.sectores_por_cluster, bs);
			printf("\nSaliendo del subdirectorio %s\n", entry.filename);
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
