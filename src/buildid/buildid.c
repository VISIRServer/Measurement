#include <stdio.h>
#include <stdlib.h>

#pragma warning( disable : 4996 )

int main(int argc, char** argv)
{
	char buffer[512];
	FILE* fp;
	int buildid = 0;

	if (argc < 2) return 1;
	fp = fopen(argv[1], "r");
	if (fp)
	{
		int rv = 0;
		rv = fscanf(fp, "%*s %*s %s", buffer);
		fclose(fp);
		if (rv > 0) buildid = atoi(buffer);
	}

	buildid++;
	fp = fopen(argv[1], "w");
	if (!fp) return 1;
	fprintf(fp, "#define BUILD_ID %i", buildid);
	fclose(fp);
	return 0;	
}
