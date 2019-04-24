#include <stdio.h>
#include <stdlib.>
#include "fatstructs.h"

//Dado un archivo (o una parte), lo busque, y si lo encuentra 
//y el mismo se encuentra borrado, lo recupere

int checkName(FILE* in, Fat12Entry entry, char[] stringBuscado)
{
	//Si no es archivo o carpeta return 0,
	if (nombreArchivo.atributos != 0x10 || nombreArchivo.atributos != 0x20)
	{
		return 0;
	}
	//Quitarle la primer letra al char[] y al nombreArchivo.filename
	if (strstr(request, "favicon") != NULL)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void recuperarArchivo(FILE* in, Fat12Entry entry, char charInicial)
{
	entry.filename[0] = charInicial;
	printf("El archivo se ha recuperado.\n");
}

void buscarEnDirectorio(FILE* in, Fat12Entry entry, int ultimoSectorLeido, int cantidadEntradas, Fat12BootSector bs, char[] archivo)
{
	int i = 0;
	for (i = 0; i < cantidadEntradas; i++) {
		fread(&entry, sizeof(entry), 1, in);
		ultimoSectorLeido = ftell(in);
		print_file_name(&entry);
		int encontrado = checkName(entry, archivo);
		if (encontrado == 1) 
		{
			printf("\nArchivo encontrado.\n");
			if(entry.filename[0] == 0xE5)
			{
				printf("\nEl archivo está eliminado.\n");
				recuperarArchivo(entry, archivo[0]);
			}
		}
		if(entry.atributos == 0x10 && entry.filename[0] != 0x2E)
		{
			unsigned int inicioDataDirectorio = (/*offset inicioData*/0x4A00 + (bs.sectores_por_cluster * bs.sector_size * (entry.cluster_inicio - 2)));
			printf("\ninicioDataDirectorio 0x%lX\n", inicioDataDirectorio);
			fseek(in, inicioDataDirectorio, SEEK_SET);
			leerDirectorio(in, entry, ftell(in), bs.sectores_por_cluster, bs);
		}
		fseek(in, ultimoSectorLeido, SEEK_SET);
	}
}

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
	
	buscarEnDirectorio(in, entry, ftell(in), bs.root_entries, bs, archivo);

        printf("\n----------\n");
	printf("\nLeido Root directory, ahora en 0x%lX\n", ftell(in));
	fclose(in);
	return 0;
}
