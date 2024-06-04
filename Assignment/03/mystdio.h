#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 1024
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


typedef struct myFile {
	int fd;
	int pos;
	int size;
	int mode;
	int flag;
	char lastop;
	int eof;	//bool
	char *buffer;
	int bufferPos;
	int bufferEnd;
} myFile;

myFile 	*myfopen(const char *pathname, const char *mode);
int 	myfread(void *ptr, int size, int nmemb, myFile *stream);
int	myfwrite(const void *ptr, int size, int nmemb, myFile *stream);
int	myfflush(myFile *stream);
int	myfseek(myFile *stream, int offset, int whence);
int	myfeof(myFile *stream);
int	myfclose(myFile *stream);

myFile *myfopen(const char *pathname, const char *mode)
{
	myFile *file = (myFile *)malloc(sizeof(myFile));	//myFile 구조체 선언 및 메모리 할당
	int flag;

	if (strcmp(mode, "r") == 0)
		flag = O_RDONLY;
	else if (strcmp(mode, "r+") == 0)
		flag = O_RDWR;
	else if (strcmp(mode, "w") == 0)
		flag = O_WRONLY | O_CREAT | O_TRUNC;
	else if (mode == "w+")
		flag = O_RDWR | O_CREAT | O_TRUNC;
	else if (mode == "a")
		flag = O_WRONLY | O_CREAT | O_APPEND;
	else if (mode == "a+")
		flag = O_RDWR | O_CREAT | O_APPEND;
	else
	{
		printf("Failed to open file (Wrong mode)\n");
		return NULL;
	}

	file->fd = open(pathname, flag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(file->fd < 0)
	{
		printf("Failed to open file (Open failed)\n");
		return NULL;
	}

	if(flag & O_APPEND)
		file->pos = lseek(file->fd, 0, SEEK_END);	//만약 file open mode가 a 또는 a+라면 마지막 커서 위치로 파일 커서 옮기기
	else
		file->pos = lseek(file->fd, 0, SEEK_SET);

	struct stat st;	//file 크기를 구하기 위한 stat 구조체 선언
	if (fstat(file->fd, &st) < 0)
	{
		printf("Failed to open file (Size error)\n");
		close(file->fd);
		free(file);
	}

	//myFile 구조체 초기화
	file->size = st.st_size;
	file->mode = flag;
	file->flag = 0;
	file->lastop = '\0';
	file->eof = 0;
	file->buffer = (char *)malloc(sizeof(char) * BUFSIZE);
	file->bufferPos = 0;
	file->bufferEnd = 0;

	return file;
}

int myfread(void *ptr, int size, int nmemb, myFile *stream)
{
	//파일 스트림 예외 처리
	if (stream == NULL || stream->mode & O_WRONLY)
	{
		printf("Failed to read file (Wrong stream)\n");
		return 0;
	}

	size_t totalSize = size * nmemb;	//read할 크기 총량
	size_t readSize = read(stream->fd, ptr, totalSize);	//read한 크기를 담기 위한 변수

	stream->pos += readSize;	//읽어들인 크기만큼 pos값 증가

	if (readSize == totalSize)	//만약 읽어들인 크기와 read할 크기 총량이 같다면 eof 처리
		stream->eof = 1;

	return (int)readSize / size;
}

int myfwrite(const void *ptr, int size, int nmemb, myFile *stream)
{
	if (stream == NULL || stream->mode & O_RDONLY)
	{
		printf("Failed to write file (Wrong stream\n");
		return 0;
	}
	size_t totalSize = size * nmemb;
	size_t writeSize = 0;

	//버퍼링 수행
	while(writeSize < totalSize)	//writeSize가 totalSize와 같거나 커질 때까지 반복
	{
		//버퍼 공간의 크기를 담기 위한 변수 선언
		size_t bufferSpace = BUFSIZE - stream->bufferPos;	//전체 버퍼 크기 대비 남은 공간 계산

		if(bufferSpace == 0)
		{
			if(myfflush(stream) == EOF)
				return writeSize / size;
			bufferSpace = BUFSIZE;
		}

		size_t copySpace = totalSize - writeSize;
		if(copySpace > bufferSpace)
			copySpace = bufferSpace;

		//stream의 버퍼로 ptr 메모리 복사
		memcpy(stream->buffer + stream->bufferPos, (const char *)ptr + writeSize, copySpace);
		stream->bufferPos += copySpace;
		writeSize += copySpace;
	}

	stream->pos += writeSize;

	return (int)writeSize / size;

}

int myfflush(myFile *stream)
{
	if(stream == NULL)
	{
		printf("Failed to flush (Wrong stream)\n");
		return EOF;
	}

	//만약 stream의 버퍼 커서가 0보다 크다면 (적어도 한번 옮겨졌다면)
	if (stream->bufferPos > 0)
	{
		size_t flushSize = write(stream->fd, stream->buffer, stream->bufferPos);
		if(flushSize != stream->bufferPos)
		{
			printf("Failed to flush\n");
			return EOF;
		}
		stream->bufferPos = 0;
	}

	return 0;
}

int myfseek(myFile *stream, int offset, int whence)
{
	if(stream == NULL)
		return -1;

	//seek하기 전에 flush 수행
	if(myfflush(stream) == EOF)
		return -1;

	//seek 수행하여 커서 이동
	off_t pos = lseek(stream->fd, offset, whence);
	if(pos < 0)
		return -1;

	//커서 관련 변수 초기화
	stream->pos = pos;
	stream->eof = 0;
	stream->bufferPos = 0;
	stream->bufferEnd = 0;

	return 0;
}

int myfeof(myFile *stream)
{
	if(stream == NULL)
		return -1;

	return stream->eof;
}

int myfclose(myFile *stream)
{
	if(stream == NULL)
		return EOF;

	//close하기 전 flush 수행
	if(myfflush(stream) == EOF)
		return EOF;

	//close 수행
	if(close(stream->fd) < 0)
	{
		printf("Failed to close file\n");
		return EOF;
	}

	//메모리 해제
	free(stream->buffer);
	free(stream);

	return 0;
}
