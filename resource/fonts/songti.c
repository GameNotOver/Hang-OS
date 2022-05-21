#include <stdio.h>

int main(void)
{
	FILE* fd = NULL;
	int i, j, k, offset;
	int flag;
	unsigned char buffer[32];
	unsigned char word[3] = {0xce, 0xe2, 0x00};
	unsigned char key[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

	fd = fopen("songti.fnt", "rb");
	if (fd == NULL)
	{
		fprintf(stderr, "error hzk16\n");
		return 1;
	}

	offset = (94 * (unsigned int)(word[0] - 0xa0 - 1) + (word[1] - 0xa0 - 1)) * 32;
	fseek(fd, offset, SEEK_SET);
	fread(buffer, 1, 32, fd);
	for (k = 0; k<32; k++){
		printf("%02X ", buffer[k]);
	}
	printf("\n");
	for (k = 0; k<16; k++)
	{
		for (j = 0; j<2; j++)
		{
			for (i = 0; i<8; i++)
			{
				flag = buffer[k * 2 + j] & key[i];
				printf("%s", flag ? "¡ñ" : "¡ð");
			}
		}
		printf("\n");
	}

	
	fclose(fd);
	fd = NULL;
	return 0;
}
