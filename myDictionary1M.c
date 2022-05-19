#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "BTreeLib/inc/btree.h"
#define SIZE_NEW_WORD 10
#define SIZE_MEANING 10
#define SIZE_DATA_SOUNDEX 50000
#define SIZE_CHAR 3

// e/'OneDrive - Hanoi University of Science and Technology'/'TAI LIEU HOC TAP'/'Code Workspace'/'Data Structure and Algorithms Advanced'/'Mini Project'
// gcc myDictionary1M.c -LBTreeLib/lib -lbt -IBTreeLib/inc

void readFILE(BTA *root, BTA *rootSdx, char *filename);
void soundex(char* name, char* s);
void soundex_insert(char *word, BTA *rootSdx);
void soundex_delete(char *word, BTA *rootSdx);
void insertWord(BTA *root, BTA *rootSdx);
void search_Suggestion(char *word, BTA *rootSdx);
void search_AutoComplete(char *word, BTA *root, BTA *rootSdx);
void searchWord(BTA *root, BTA *rootSdx);
void deleteWord(BTA *root, BTA *rootSdx);

// Hàm xử lý chuỗi kí tự
char *getString(char *buf, size_t size){
    if (buf != NULL && size > 0){
        if (fgets(buf, size, stdin)){
            buf[strcspn(buf, "\n")] = '\0';
            return buf;
        }
        *buf = '\0';
    }
    return NULL;
}

// MENU
int menu(){
    char choice[SIZE_CHAR];
    printf("\n________________________\n");
    printf("__________MENU__________\n\n");
    printf("1. Search\n");
    printf("2. Insert Word\n");
    printf("3. Delete Word\n");
    printf("4. Exit\n");
    printf("\n");
    printf("Enter your choice: ");
    getString(choice, sizeof choice);
    printf("________________________\n\n");
    if (strcmp(choice, "1") == 0) return 1;
    else if (strcmp(choice, "2") == 0) return 2;
    else if (strcmp(choice, "3") == 0) return 3;
    else return 4;
}

// Đọc file .txt sang file .db
void readFILE(BTA *root, BTA *rootSdx, char *filename){
    char newWord[SIZE_NEW_WORD];
    char meaning[SIZE_MEANING];
    int i = 0; /* count the number of words is loaded into the dictionary */
    FILE *fin = fopen(filename, "r");
    if (fin == NULL){
        printf("Can NOT Open File\n");
        exit(1);
    }
    while (!(feof(fin)))
    {
        fscanf(fin, "%s %[^\n]", newWord, meaning);
        int result = btins(root, newWord, meaning, SIZE_MEANING);
        if (result == 0){
            soundex_insert(newWord, rootSdx);
            i+=1;
        }
    }
    printf("(%d) words are loaded into the dictionary\n", i);
    printf("____________________________________________________________\n");
}

// Thêm từ mới vào từ điển. Nếu từ tồn tại thì có thể cập nhật nghĩa mới
void insertWord(BTA *root, BTA *rootSdx){
    char newWord[SIZE_NEW_WORD];
    char newMeaning[SIZE_MEANING];
    int rsize;
    printf("Enter New Word: ");
    getString(newWord, sizeof newWord);
    if (btsel(root, newWord, newMeaning, SIZE_MEANING, &rsize) == 0){
        printf("'%s' ALREADY exists\nDo you want to update the new meaning? (Y/N): ", newWord);
        char yes[SIZE_CHAR];
        getString(yes, sizeof yes);
        if (strcmp(yes, "y") == 0 || strcmp(yes, "Y") == 0){
            printf("Enter New Meaning: ");
            getString(newMeaning, sizeof newMeaning);
            int upd = btupd(root, newWord, newMeaning, SIZE_MEANING);
            if (upd == 0) printf("Updated New Meaning\n");
            else printf("Error %d\n", upd);
        }
    }
    else {
        printf("Enter Meaning: ");
        getString(newMeaning, sizeof newMeaning);
        int flag = btins(root, newWord, newMeaning, SIZE_MEANING);
        if (flag == 0){
            soundex_insert(newWord, rootSdx);
            printf("'%s' is added\n", newWord);
        }
        else printf("Error %d\n", flag);
    }
}

// Tìm kiếm đề xuất cho từ nhập sai
void search_Suggestion(char *word, BTA *rootSdx){
    char sdx[] = "     ";
    char sdx_data[SIZE_DATA_SOUNDEX];
    int rsize_sdx;
    soundex(word, sdx);
    int i = btsel(rootSdx, sdx, sdx_data, SIZE_DATA_SOUNDEX, &rsize_sdx);
    if (i == 0){
        printf("Did you mean:\n");
        printf("_____________\n");
        printf("%s\n", sdx_data);
    }
    else printf("Try Again\n");
}

// Tự động hoàn thành tìm kiếm. Bấm tab để kích hoạt
void search_AutoComplete(char *word, BTA *root, BTA *rootSdx){
    char autoWord[SIZE_NEW_WORD];
    char autoMeaning[SIZE_MEANING];
    char sdx[] = "     ";
    char sdx_data[SIZE_DATA_SOUNDEX];
    int rsize;
    int count = 0, max = 0;
    char *token;
    soundex(word, sdx);
    int i = btsel(rootSdx, sdx, sdx_data, SIZE_DATA_SOUNDEX, &rsize);
    if (i == 0){
        token = strtok(sdx_data, "\n");
        while (token)
        {
            // Kiểm tra số kí tự giống nhau của chuỗi nhập vào và chuỗi đưa ra gợi ý
            // Ưu tiên chuỗi giống nhất với chuỗi nhập vào
            for (int j = 0; j < strlen(word); j++){
                if (word[j] == token[j]) count += 1;
                else break;
            }
            if (count > max){
                max = count;
                strcpy(autoWord, token);
            }
            else if (count == max){
                if (strlen(token) < strlen(autoWord)) strcpy(autoWord, token);
            }
            count = 0;
            token = strtok(NULL, "\n");
        }
        free(token);
        int sel = btsel(root, autoWord, autoMeaning, SIZE_MEANING, &rsize);
        if (sel == 0) printf("Word: %s\nMeaning: %s\n", autoWord, autoMeaning);
    }
    else printf("Try Again\n");
}

// searchWord = search_Suggestion + search_AutoComplete
void searchWord(BTA *root, BTA *rootSdx){
    char tempWord[SIZE_NEW_WORD];
    char tempMeaning[SIZE_MEANING];
    int rsize;
    int flag = 0;
    printf("Enter Word: ");
    getString(tempWord, sizeof tempWord);
    // Kiểm tra xem có bấm phím Tab không
    if (tempWord[strlen(tempWord) - 1] == '\t') flag = 1;
    // Tìm kiếm từ
    int sel = btsel(root, tempWord, tempMeaning, SIZE_MEANING, &rsize);
    if (sel == 0) printf("Word: %s\nMeaning: %s\n", tempWord, tempMeaning);
    // Nếu không tồn tại từ thì đưa ra gợi ý hoặc tự động điền
    else {
        if (flag == 1){
            search_AutoComplete(tempWord, root, rootSdx);
        }
        else {
            search_Suggestion(tempWord, rootSdx);
        }
    }
}

// Xóa từ khỏi từ điển
void deleteWord(BTA *root, BTA *rootSdx){
    char delWord[SIZE_NEW_WORD];
    char delMeaning[SIZE_MEANING];
    int rsize;
    printf("Enter Delete Word: ");
    getString(delWord, sizeof delWord);
    int del = btdel(root, delWord);
    if (del == 0){
        printf("'%s' is deleted\n", delWord);
        soundex_delete(delWord, rootSdx);
    }
    else printf("'%s' NOT exists\n", delWord);
}

// Hàm chuyển đổi 1 từ sang mã soundex tương ứng
void soundex(char* name, char* s){
    int si = 1;
    char c;

    //                 ABCDEFGHIJKLMNOPQRSTUVWXYZ
    char mappings[] = "01230120022455012623010202";
    s[0] = toupper(name[0]);

    for (int i = 1, l = strlen(name); i < l; i++)
    {
        c = toupper(name[i]) - 65;

        if (c >= 0 && c <= 25)
        {
            if (mappings[c] != '0')
            {
                if (mappings[c] != s[si-1])
                {
                    s[si] = mappings[c];
                    si++;
                }

                if (si > 3)
                {
                    break;
                }
            }
        }
    }

    if (si <= 3)
    {
        while(si <= 3)
        {
            s[si] = '0';
            si++;
        }
    }
}

// Thêm mã soundex khi thêm từ
void soundex_insert(char *word, BTA *rootSdx){
    char sdx[] = "     ";
    char sdx_data[SIZE_DATA_SOUNDEX];
    int rsize;
    soundex(word, sdx);
    int flag = btsel(rootSdx, sdx, sdx_data, SIZE_DATA_SOUNDEX, &rsize);
    // Nếu chưa tồn tại mã soundex thì thêm mã mới vào database
    if (flag != 0){
        int ins = btins(rootSdx, sdx, word, SIZE_NEW_WORD);
        if (ins != 0) printf("Error %d\n", ins);
    }
    // Nếu đã tồn tại thì cập nhật thêm từ vào dữ liệu của mã soundex
    else {
        strcat(sdx_data, "\n");
        strcat(sdx_data, word);
        btupd(rootSdx, sdx, sdx_data, SIZE_DATA_SOUNDEX);
    }
}

// Xóa dữ liệu trong soundex khi xóa từ
void soundex_delete(char *word, BTA *rootSdx){
    char sdx[] = "     ";
    char sdx_data[SIZE_DATA_SOUNDEX];
    int rsize;
    char *token;
    soundex(word, sdx);
    int flag = btsel(rootSdx, sdx, sdx_data, SIZE_DATA_SOUNDEX, &rsize);
    if (flag == 0){
        if (strcmp(sdx_data, word) == 0){
            int del = btdel(rootSdx, sdx);
            if (del != 0) printf("Error %d\n", del);
        }
        else {
            token = strtok(sdx_data, "\n");
            int del = btdel(rootSdx, sdx);
            if (del != 0) printf("Error %d\n", del);
            while (token)
            {
                if (strcmp(token, word) != 0){
                    soundex_insert(token, rootSdx);
                }
                token = strtok(NULL, "\n");
            }
            free(token);
        }
    }
}

int main(){
    char db_dictionary[] = "1M_Words.db";
    char db_soundex[] = "1M_Words_Soundex.db";
    char file_name[] = "Text1M.txt";
    BTA *root = btopn(db_dictionary, 0, 0);
    BTA *rootSdx = btopn(db_soundex, 0, 0);

    if (root == NULL && rootSdx == NULL){
        clock_t start=clock();
        root = btcrt(db_dictionary, 0, 0);
        rootSdx = btcrt(db_soundex, 0, 0);
        readFILE(root, rootSdx, file_name);
        clock_t end=clock();
        clock_t tick = end - start;
        double time_in_second=(double)tick/CLOCKS_PER_SEC;
        printf("Time: %lf\n", time_in_second);
    }

    do {
        int choice = menu();
        switch (choice)
        {
        case 1:
        {
            clock_t start1=clock();
            searchWord(root, rootSdx);
            clock_t end1=clock();
            clock_t tick1 = end1 - start1;
            double time_in_second_1=(double)tick1/CLOCKS_PER_SEC;
            printf("Time: %lf\n", time_in_second_1);
            break;
        }
        case 2:
        {
            clock_t start2=clock();
            insertWord(root, rootSdx);
            clock_t end2=clock();
            clock_t tick2 = end2 - start2;
            double time_in_second_2=(double)tick2/CLOCKS_PER_SEC;
            printf("Time: %lf\n", time_in_second_2);
            break;
        }
        case 3:
            deleteWord(root, rootSdx);
            break;
        case 4:
            btcls(root);
            btcls(rootSdx);
            exit(1);
            break;
        default:
            btcls(root);
            btcls(rootSdx);
            exit(1);
            break;
        }
    } while (1);
}