#include <stdlib.h>
#include <stdio.h>

#include "commands.h"
#include "entries.h"
#include "utils.h"


void commandCreate(void){
    char* csv_input_name;
    char* bin_output_name;
    scanf("%ms %ms", &csv_input_name, &bin_output_name);

    FILE* fp_in;
    FILE* fp_out;
    OPEN_FILE(fp_in, csv_input_name, "rb");
    OPEN_FILE(fp_out, bin_output_name, "wb");



    fclose(fp_in);
    fclose(fp_out);
    free(csv_input_name);
    free(bin_output_name);
}

void commandFrom(void){
    char* bin_filename;
    scanf("%ms", &bin_filename);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb");


    entry* e = readEntry(fp);
    printEntry(e);

    FILE* fp2;
    OPEN_FILE(fp2, "out.out", "wb");
    writeEntry(fp2, e);

    fclose(fp2);
    deleteEntry(e);
    fclose(fp);
    free(bin_filename);
}

void commandWhere(void){
    char* bin_filename;
    int n;
    scanf("%ms %d", &bin_filename, &n);
    //readTuples(n);

    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rb");



    fclose(fp);
    free(bin_filename);
}

void commandDelete(void){
    char* bin_filename;
    int n;
    scanf("%ms %d", &bin_filename, &n);
    //readTuples(n);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rwb");

    

    fclose(fp);
    free(bin_filename);
}

void commandInsert(void){
    char* bin_filename;
    int n;
    scanf("%ms %d", &bin_filename, &n);
    //readTable(n);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rwb");

    

    fclose(fp);
    free(bin_filename);
}

void commandCompact(void){
    char* bin_filename;
    scanf("%ms", &bin_filename);
    
    FILE* fp;
    OPEN_FILE(fp, bin_filename, "rwb");

    

    fclose(fp);
    free(bin_filename);
}