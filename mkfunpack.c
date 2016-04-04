#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MKF_FILE "midi.mkf"
#define UNPACK_NUMBERSTART 1
#define UNPACK_TEMPLATE "%03d.MID"

void write_to_file(int number, char *data, int size)
{
    FILE *fp;
    char filename[128];
    int ret;

    // construct filename
    snprintf(filename, sizeof(filename), UNPACK_TEMPLATE, number + UNPACK_NUMBERSTART);
    
    if (size <= 0) {
        printf("file %s is empty!\n", filename);
        return;
    }
    
    // real write
    printf("write to file %s ...\n", filename);
    fp = fopen(filename, "wb");
    assert(fp != NULL);
    ret = fwrite(data, 1, size, fp);
    assert(ret == size);
    fclose(fp);
}

int main()
{
    FILE *fp;
    int i;
    int ret;
    int header_size;
    int mkf_length;
    int file_count;
    int *file_size;
    char *buf;
    
    printf("MKF_FILE = %s\n", MKF_FILE);
    printf("UNPACK_TEMPLATE = %s\n", UNPACK_TEMPLATE);
    
    // open file
    fp = fopen(MKF_FILE, "rb");
    assert(fp != NULL);
    
    // find out the length of mkf file
    ret = fseek(fp, 0, SEEK_END);
    assert(ret == 0);
    mkf_length = ftell(fp);
    assert(mkf_length > 0);
    ret = fseek(fp, 0, SEEK_SET);
    assert(ret == 0);
    printf("mkf_length = %d\n", mkf_length);
    
    // read mkf header
    ret = fread(&header_size, sizeof(int), 1, fp);
    assert(ret == 1);
    file_count = header_size / sizeof(int) - 1;
    assert(file_count > 0);
    printf("header_size = 0x%08X, file_count = %d\n", header_size, file_count);
    
    // read file offset table
    file_size = malloc(file_count * sizeof(int));
    assert(file_size != NULL);
    ret = fread(file_size, sizeof(int), file_count, fp); // file_size is file_offset here
    assert(ret = file_count);
    
    // fixup file_offset to file_size
    assert(file_size[0] == header_size);
    for (i = 0; i < file_count - 1; i++) {
        file_size[i] = file_size[i + 1] - file_size[i];
        assert(file_size[i] >= 0);
    }
    file_size[file_count - 1] = mkf_length - file_size[file_count - 1];
    assert(file_size[file_count - 1] >= 0);
    
    // read files, read whole file in one block
    for (i = 0; i < file_count; i++) {
        //printf("file_size[%03d] = 0x%08X\n", i, file_size[i]);
        
        buf = malloc(file_size[i]);
        assert(buf != NULL);
        
        ret = fread(buf, 1, file_size[i], fp);
        assert(ret == file_size[i]);
        
        write_to_file(i, buf, file_size[i]);
        
        free(buf);
    }
    
    // clean up
    free(file_size);
    fclose(fp);
    
    puts("OK");
    system("pause");
    return 0;
}
