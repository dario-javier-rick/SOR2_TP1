#include <stdio.h>
#include <stdlib.h>

int main() {
	FILE * in = fopen("test.img", "rb");
	unsigned int i, start_sector, length_sectors;

	fseek(in /*stream*/, 0x1BE /*offset*/, SEEK_SET /*principio del stream*/); // Voy al inicio... 

	for (i = 0; i < 4; i++) { // Leo las entradas. El MBR puede almacenar hasta 4 particiones primarias, por eso el for es hasta 4
		printf("Partition entry %d: First byte %02X\n", i, fgetc(in));
		printf("  Partition start in CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));

		unsigned char tipo_particion = fgetc(in);

		printf("  Partition type %02X\n", tipo_particion);
		switch (tipo_particion)
		{
		case 0x01:
			printf(" FAT12 \n");
			break;
		case 0x04:
			printf(" FAT16 menor a 32mb \n");
			break;
		case 0x06:
			printf(" FAT16 mayor a 32mb \n");
			break;
		case 0x07:
			printf(" NTFS \n");
			break;
		case 0x08:
			printf(" FAT32 (DOS/Win95) \n");
			break;
		case 0x0C:
			printf(" FAT32 modo LBA particion FAT32 \n");
			break;
		case 0x0E:
			printf(" FAT32 modo LBA particion FAT16 \n");
			break;
		default:
			printf(" Tipo de particion no reconocida \n");
		}

		printf("  Partition end in CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));

		printf("  start_sector %d\n", start_sector);
		printf("  length_sectors %d\n", length_sectors);
		fread(&start_sector, 4, 1, in);
		fread(&length_sectors, 4, 1, in);
		printf("  Relative LBA address 0x%08X, %d sectors long\n", start_sector, length_sectors);
	}

	fclose(in);
	return 0;
}
