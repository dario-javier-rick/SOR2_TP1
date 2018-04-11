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

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    
    fseek(in, 0x1BE, SEEK_SET); // Ir al inicio de la tabla de particiones
    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas 
    
    for(i=0; i<4; i++) {        
        printf("Partition type: %d\n", pt[i].partition_type);
        if(pt[i].partition_type == 1) {
            printf("Encontrado FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No FAT12 filesystem found, exiting...\n");
        return -1;
    }
    
    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("  Jump code: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);
    printf("  OEM code: [%.8s]\n", bs.oem);
    printf("  sector_size: %d\n", bs.sector_size);
    printf("  sectores por cluster: %d\n", bs.sectores_por_cluster);
    printf("  sectores reservados: %d\n", bs.sectores_reservados);
    printf("  numero de tablas fat: %d\n ", bs.cantidad_tablas_fats);
	// {...} COMPLETAR
//    printf("  volume_id: 0x%08X\n", (unsigned int)bs.volume_id);
    printf("  Volume label: [%.11s]\n", bs.volume_label);
    printf("  Filesystem type: [%.8s]\n", bs.fs_type);
    printf("  Boot sector signature: 0x%04X\n", bs.boot_sector_signature);
    
    fclose(in);
    return 0;
}
