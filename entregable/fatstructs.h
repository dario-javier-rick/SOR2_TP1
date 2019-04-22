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
