#include <stdio.h>
#include <stdlib.>
#include "fatstructs.h"

//Dado un archivo (o una parte), lo busque, y si lo encuentra 
//y el mismo se encuentra borrado, lo recupere

int main(int argc, char *argv[]) {

	char imagen[] = argv[1]; //Filesystem en formato .img
	char archivo[] = argv[2]; //Archivo a recuperar

	FILE * in = fopen(imagen, "rb");
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


        //Voy al final de la ultima tabla FAT, donde comienza el root directory
	fseek(in, (bs.sectores_reservados - 1 + bs.tamanio_fat * bs.cantidad_tablas_fats) *
		bs.sector_size, SEEK_CUR);
        printf("\nInicio Root directory en 0x%lX\n", ftell(in));


	printf("Root dir_entries %d \n", bs.root_entries);
	for (i = 0; i < bs.root_entries; i++) {
                //printf("\nAhora en 0x%lX\n", ftell(in));
		fread(&entry, sizeof(entry), 1, in);
                unsigned int ultimoSectorLeido = ftell(in);

		//print_file_info(in, &entry, sizeof(entry), &bs, i);
		if(&entry->filename[0] == "0xE5"){ //Si es un archivo borrado..
			//TODO: Me fijo si es = a archivo 
			//TODO: si es igual, cambio el 0xE5 por 0x05
		}

                fseek(in, ultimoSectorLeido, SEEK_SET);
	}

        printf("\n----------\n");
	printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
	fclose(in);
	return 0;
}
