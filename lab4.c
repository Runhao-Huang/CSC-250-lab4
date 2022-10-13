/**
 * This program reads a WAV audio file and prints statistics about the audio samples. The file name 
 * is provided using command line arguments. If the file name is not provided or the file is not readable, 
 * the program will exit and provide an error message.
 *
 * @author Runhao Huang huanr20@wfu.edu
 * @date Oct. 2, 2022
 * @assignment Lab 4  
 * @course CSC 250
 **/

#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  
#include <math.h>  


int readWavReader(FILE* inFile, short *sampleSizePtr, int *numSamplesPtr, int *sampleRatePtr, short *numOfChannel);
int readWavData(FILE* inFile, short sampleSize, int numSamples, int sampleRate, short numOfChannel);

/*
* The main function contains the validation of the wav file through argument number and opening the wav file. After validation,
* it calls the function that handles the chunks in the file before data chunk and print out information of the file. If the format is 
* correct, it will then call the function that reads all the data in the file data chunk and print the largest absolute value of the channels.
*/

int main(int argc, char *argv[]) {
    FILE *inFile;      /* WAV file */
    short sampleSize;  /* size of an audio sample (bits) */
    int sampleRate;    /* sample rate (samples/second) */
    int numSamples;    /* number of audio samples */ 
    int wavOK = 0;     /* 1 if the WAV file si ok, 0 otherwise */
    short numOfChannel; /* Number of channels in the file. */

    /* Check the argument number. */
    if(argc < 2) {
        printf("usage: %s wav_file \n", argv[0]);
        return 1;
    }

    /* Try to read the file. */
    inFile = fopen(argv[1], "rbe"); 
    if(!inFile) {
        printf("could not open wav file %s \n", argv[1]);
        return 2;
    }

    /* Call the function to read the chuncks before data chunk. */
    wavOK = readWavReader(inFile, &sampleSize, &numSamples, &sampleRate, &numOfChannel);
    if(!wavOK) {
       printf("wav file %s has incompatible format \n", argv[1]);   
       return 3;
    }
    else
        /* Call the funtion to read the data chunck. */
        readWavData(inFile, sampleSize, numSamples, sampleRate, numOfChannel);

    if(inFile) fclose(inFile);
    return 0;
}


/**
 *  function reads the RIFF, fmt, and start of the data chunk. 
 */
int readWavReader(FILE* inFile, short *sampleSizePtr, int *numSamplesPtr, int *sampleRatePtr, short *numOfChannel) {
    char chunkId[] = "    ";  /* chunk id, note initialize as a C-string */
    char data[] = "    ";      /* chunk data */
    int chunkSize = 0;        /* number of bytes remaining in chunk */
    short audioFormat = 0;    /* audio format type, PCM = 1 */
    short numChannels = 0;    /* number of audio channels */ 
    int sampleRate = 0;       /* Audio samples per second */ 
    int byteRate = 0;         /* SampleRate × NumChannels × BitsPerSample/8 */ 
    short blockAlign = 0;     /* The number of bytes for one sample including all channels. */
    short bitsPerSample = 0;  /* Number of bits used for an audio sample. */
    int numSampleByte = 0;    /* This is the number of bytes of data (audio samples). */

    /* first chunk is the RIFF chunk, let's read that info */  
    fread(chunkId, 1, 4, inFile);
    fread(&chunkSize, 1, 4, inFile);
    printf("chunk: %s\n", chunkId);
    fread(data, 1, 4, inFile);
    printf("  data: %s \n", data);

    /* let's try to read the next chunk, it always starts with an id */
    fread(chunkId, 1, 4, inFile);
    /* if the next chunk is not "fmt " then let's skip over it */  
    while(strcmp(chunkId, "fmt ") != 0) {
        fread(&chunkSize, 1, 4, inFile);
        /* skip to the end of this chunk */  
        fseek(inFile, chunkSize, SEEK_CUR);
        /* read the id of the next chuck */  
        fread(chunkId, 1, 4, inFile);
    }  

    /* if we are here, then we must have the fmt chunk, now read that data */  
    fread(&chunkSize, 1, 4, inFile);
    fread(&audioFormat, 1,  sizeof(audioFormat), inFile);
    fread(&numChannels, 1,  sizeof(numChannels), inFile);
    fread(&sampleRate, 1,  sizeof(sampleRate), inFile);
    fread(&byteRate, 1,  sizeof(byteRate), inFile);
    fread(&blockAlign, 1,  sizeof(blockAlign), inFile);
    fread(&bitsPerSample, 1,  sizeof(bitsPerSample), inFile);
    

    printf("chunk: %s\n", chunkId);
    printf(" audio Format: %d \n", audioFormat);
    printf(" num channels: %d \n", numChannels);
    printf(" sample rate: %d \n", sampleRate);
    printf(" bits per sample: %d \n", bitsPerSample);


    /* let's try to read the next chunk, it always starts with an id */
    fread(chunkId, 1, 4, inFile);
    /* if the next chunk is not "data" then let's skip over it */  
    while(strcmp(chunkId, "data") != 0) {
        fread(&chunkSize, 1, 4, inFile);
        /* skip to the end of this chunk */  
        fseek(inFile, chunkSize, SEEK_CUR);
        /* read the id of the next chuck */  
        fread(chunkId, 1, 4, inFile);
    } 

    /* Read the size of the data chunk here */
    fread(&numSampleByte, 1, 4, inFile);
    *numSamplesPtr = (numSampleByte*8/numChannels)/bitsPerSample;
    *sampleSizePtr = (numSampleByte/(*numSamplesPtr))*8;
    *sampleRatePtr = sampleRate;
    *numOfChannel = numChannels;

    printf("chunk: %s\n", chunkId);
    printf(" num samples: %d \n", *numSamplesPtr);
    printf(" duration: %f (sec)\n", ((double)*numSamplesPtr)/sampleRate);

    return (audioFormat == 1);
}


/**
 *  function reads the WAV audio data (last part of the data chunk)
 */
int readWavData(FILE* inFile, short sampleSize, int numSamples, int sampleRate, short numOfChannel) {
    int sample = 0;
    int bytes_per_sample = sampleSize / (8 * numOfChannel);
    int leftMax = 0;
    int rightMax = 0;
    int i;
    
    if (numOfChannel==2) {
        numSamples *= 2;
    }
    /* Loop through all the channels in the samples and get the maximum absolute value of the channels. */
    for (i = 0; i < numSamples; i++) {
        fread(&sample, 1, bytes_per_sample, inFile);
        if(bytes_per_sample == 2) {
            sample = abs((short) sample);
        }
        else if(bytes_per_sample == 4) {
            sample = abs((int) sample);
        }
        if ((i % 2) == 0 && sample>leftMax) {
            leftMax = sample;
        }
        else if ((i % 2) == 1 && sample>rightMax) {
            rightMax = sample;
        }
    }

    /* Print out the result of maximum aboslute value of the channels here. */
    if (numOfChannel == 1) {
        if (leftMax > rightMax) {
            printf(" max abs mono sample: %d\n", leftMax);
        }
        else {
            printf(" max abs mono sample: %d\n", rightMax);
        }
    }
    else {
        printf(" max abs left sample:  %d\n", leftMax);
        printf(" max abs right sample: %d\n", rightMax);
    }

    return 1;
}


