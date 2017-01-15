// filehdr.cc
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector,
//
//      Unlike in a real system, we do not keep track of file permissions,
//	ownership, last modification date, etc., in the file header.
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::FileHeader
//	There is no need to initialize a fileheader,
//	since all the information should be initialized by Allocate or FetchFrom.
//	The purpose of this function is to keep valgrind happy.
//----------------------------------------------------------------------
FileHeader::FileHeader()
{
	/* MP4 */
	nextFileHeaderSector = -1;
	nextFileHeader = NULL;

	numBytes = -1;
	numSectors = -1;
	memset(dataSectors, -1, sizeof(dataSectors));
}

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::~FileHeader
//	Currently, there is not need to do anything in destructor function.
//	However, if you decide to add some "in-core" data in header
//	Always remember to deallocate their space or you will leak memory
//----------------------------------------------------------------------
FileHeader::~FileHeader()
{
	/* MP4 */
	if(nextFileHeader != NULL)
		delete nextFileHeader; /* invoke destructor of nextFileHeader recursively */
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{
	/* MP4 */
	/* How much data bytes this header has? */
	int remainFileSize = fileSize - MaxFileSize;
	if(remainFileSize <= 0)/* We don't need next header */
		numBytes = fileSize;
	else  /* We need next header */
		numBytes = MaxFileSize;


    numSectors  = divRoundUp(numBytes, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    for (int i = 0; i < numSectors; i++)
	{
		dataSectors[i] = freeMap->FindAndSet();
		// since we checked that there was enough free space,
		// we expect this to succeed
		ASSERT(dataSectors[i] >= 0);

		/* MP4 */
		/* Clean this sector */
		char tmp[SectorSize];	for(int i=0 ; i<SectorSize ; i++)	tmp[i] = 0;
		kernel->synchDisk->WriteSector(dataSectors[i], tmp);
		

    }

	/* MP4 */
	/* Need next header */
	if(remainFileSize > 0)
	{
		/* Make a new header */
		nextFileHeaderSector = freeMap->FindAndSet();
		if(nextFileHeaderSector == -1)	return FALSE;

		nextFileHeader = new FileHeader;
		return nextFileHeader->Allocate(freeMap, remainFileSize);
	}

    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void
FileHeader::Deallocate(PersistentBitmap *freeMap)
{
	/* MP4 */
	/* Deallocate recursively */
	if(nextFileHeader != NULL)	nextFileHeader->Deallocate(freeMap);

	for (int i = 0; i < numSectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk.
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{

	/* MP4 */
	char buf[SectorSize];
	kernel->synchDisk->ReadSector(sector, buf);

	/* dick head */
	int offset = sizeof(numBytes);
	memcpy(&numBytes, buf, sizeof(numBytes));
	memcpy(&numSectors, buf + offset, sizeof(numSectors));	offset += sizeof(numSectors);
	memcpy(&nextFileHeaderSector, buf + offset, sizeof(nextFileHeaderSector)); offset += sizeof(nextFileHeaderSector);
	memcpy(dataSectors, buf + offset, NumDirect * sizeof(int));

	/* MP4 */
	/* Fetch nextFileHeader */
	if(nextFileHeaderSector != -1)
	{
		nextFileHeader = new FileHeader;
		nextFileHeader->FetchFrom(nextFileHeaderSector);
	}

	/*
		MP4 Hint:
		After you add some in-core informations, you will need to rebuild the header's structure
	*/

}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk.
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{


	/* MP4 */
	char buf[SectorSize];
	int offset = sizeof(numBytes);

	/* We don;t need to copy in-core data to disk */
	memcpy(buf , &numBytes, sizeof(numBytes));
	memcpy(buf + offset, &numSectors, sizeof(numSectors)); offset += sizeof(numSectors);
	memcpy(buf + offset, &nextFileHeaderSector, sizeof(nextFileHeaderSector)); offset += sizeof(nextFileHeaderSector);
	memcpy(buf + offset, dataSectors, NumDirect*sizeof(int));
	kernel->synchDisk->WriteSector(sector, buf);

	/* WriteBack nextFileHeader */
	if(nextFileHeaderSector != -1)
		nextFileHeader->WriteBack(nextFileHeaderSector);

	/*
		MP4 Hint:
		After you add some in-core informations, you may not want to write all fields into disk.
		Use this instead:
		char buf[SectorSize];
		memcpy(buf + offset, &dataToBeWritten, sizeof(dataToBeWritten));
		...
	*/

}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
	/* MP4 */
	int index = offset / SectorSize;
	if(index < NumDirect)
		return(dataSectors[index]);
	else
		return nextFileHeader->ByteToSector(offset - MaxFileSize);
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
	/* MP4 */
	int totalNumBytes = numBytes;
	if(nextFileHeader != NULL)
		totalNumBytes += nextFileHeader->FileLength();
	return totalNumBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	kernel->synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n");
    }

	/* MP4 */
	if(nextFileHeader != NULL)
		nextFileHeader->Print();

    delete [] data;
}
