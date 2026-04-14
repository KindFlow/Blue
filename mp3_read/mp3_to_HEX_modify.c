#include <stdio.h>
#include <stdlib.h>

unsigned char* readFile(const char* filename, size_t* size);
void binaryToHex(const unsigned char* data, size_t size, char* output);

size_t j=0;
int main()
{

	const char* filename = "D:\\5.HelpDoc(Software)\\study\\TI Bluetooth\\Mine\\Bluetooth_Music\\mp3_read\\DD.mp3"; // MP3文件路径
	const char* filename_write2 = "D:\\5.HelpDoc(Software)\\study\\TI Bluetooth\\Mine\\Bluetooth_Music\\mp3_read\\DD_to_piece.txt"; // 16进制字符输出到文件
	size_t size;
    unsigned char* data = readFile(filename, &size);
    if (!data) {
        return 1;
    }


    // char* hexString = (char*)malloc(size * 2 + 1);
	char* hexString = (char*)malloc(size * 2 + (size * 2) + 1);
    if (!hexString) {
        perror("Failed to allocate memory for hex string");
        free(data);
        return 1;
    }

    binaryToHex(data, size, hexString);

#if 0
    printf("Hexadecimal representation:\n%s\n", hexString);
#else
	FILE* file_write = fopen(filename_write2, "wb");	//only write
	
	if (!file_write) {
		perror("Failed to open file");
		return NULL;
	}
	if (file_write != NULL)
	{
		fprintf(file_write, "%s", hexString);
		fclose(file_write);
	}


#endif

    free(data);
    free(hexString);
    return 0;
}


unsigned char* readFile(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);		//uint: byte
    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = (unsigned char*)malloc(*size);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *size, file);	//1 represent 1 byte
    fclose(file);
    return buffer;
}


void binaryToHex(const unsigned char* data, size_t size, char* output) {
    for (size_t i = 0; i < size; ++i)
	{
		if(i==0)
		{}
		else if(i%2 == 0)
		{
			sprintf(output + i*2 + j, "%c", '\t');
			j++;
			sprintf(output + i*2 + j, "%c", '\n');
			j++;
		}

        sprintf(output + i * 2 + j, "%02X", data[i]);

    }
    output[size * 2] = '\0'; // Null-terminate the string
}
