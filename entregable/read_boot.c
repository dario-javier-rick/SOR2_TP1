#include <stdio.h>
#include <stdlib.h>
#include "fatstructs.h"

char* obtenerMediaType(unsigned char str) {
    switch(str) {
      case 0xF8:
        return "Fixed Disk";
      case 0xF0:
        return "Removable Disk";
      default:
        return "N/A";
    }
}

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
    printf("  Tamaño de sectores: %d\n", bs.sector_size);
    printf("  Sectores por cluster: %d\n", bs.sectores_por_cluster);
    printf("  Sectores reservados: %d\n", bs.sectores_reservados);
    printf("  Cantidad tablas fat: %d\n", bs.cantidad_tablas_fats);
    printf("  Root entries: %d\n", bs.root_entries);
    printf("  Cantidad sectores: %d\n", bs.cantidad_sectores);
    printf("  Media type: [%s]\n", obtenerMediaType(bs.media_descriptor[0]));
    printf("  Tamaño fat: %d\n", bs.tamanio_fat);
    printf("  Sectores por track: %d\n", bs.sectores_por_track);
    printf("  Cantidad heads: %d\n", bs.cantidad_heads);
    printf("  Sectores ocultos: %i\n", bs.sectores_ocultos);
    printf("  Sectores filesystem: %i\n", bs.sectores_filesystem);
    printf("  Volume label: [%.11""s]\n", bs.volume_label);
    printf("  Filesystem type: [%.8s]\n", bs.fs_type);
    printf("  Boot sector signature: 0x%04X\n", bs.boot_sector_signature);

    fclose(in);
    return 0;
}
