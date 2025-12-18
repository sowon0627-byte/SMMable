//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"


//board configuration parameters
static int smm_board_nr;
static int smm_food_nr;
static int smm_festival_nr;
static int smm_player_nr;

//structure type definition
typedef struct {
        char name[MAX_CHARNAME];
        int pos;
        int credit;
        int energy;
        int flag_graduated;
} smm_player_t;


smm_player_t *smm_players;



void generatePlayers(int n, int initEnergy); //generate a new player
void printPlayerStatus(void); //print all player status at the beginning of each turn

//function prototypes
#if 0
void printGrades(int player); //print grade history of the player
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void printGrades(int player); //print all the grade history of the player
#endif




void* findGrade(int player, char *lectureName) //find the grade from the player's grade history
{
      int size = smmdb_len(LISTNO_OFFSET_GRADE + player);
      int i;
      
      for (i=0;i<size;i++)
      {
          void *ptr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
          if (strcmp(smmObj_getObjectName(ptr), lectureName) == 0)
          {
                                                return ptr;                    
          }
          
      }
      
      return NULL;    
}


int isGraduated(void) // check if any player is graduated
{
    int i;
    for (i=0;i<smm_player_nr;i++)
    {
        if (smm_players[i].flag_graduated == 1)
           return 1;
    }
    
    return 0;
}
    




void goForward(int player, int step) //make player go "step" steps on the board (check if player is graduated)
{
     int i;
     void *ptr;
     
     
     //player_pos[player] = player_pos[player] + step;
     ptr = smmdb_getData(LISTNO_NODE,smm_players[player].pos);
     printf("start from %i(%s) (%i)\n", smm_players[player].pos, smmObj_getObjectName(ptr), step);
     
     for(i=0;i<step;i++)
     {
                        smm_players[player].pos = (smm_players[player].pos + 1)%smm_board_nr;
                        
                        ptr = smmdb_getData(LISTNO_NODE,smm_players[player].pos);
                        
                        printf(" => moved to %i(%s)\n", smm_players[player].pos, smmObj_getObjectName(ptr));
     }
 }



void printPlayerStatus(void) //print all player status at the beginning of each turn
{
    int i;
    void *ptr;
    
    for (i=0;i<smm_player_nr;i++)
    {
        ptr = smmdb_getData(LISTNO_NODE,smm_players[i].pos);
        
        printf("%s - position:%i(%s), credit:%i, energy:%i\n",
                  smm_players[i].name, smm_players[i].pos, smmObj_getObjectName(ptr), smm_players[i].credit, smm_players[i].energy);
    }
}



void generatePlayers(int n, int initEnergy) //generate a new player
{
    int i;
    
    smm_players = (smm_player_t*)malloc(n*sizeof(smm_player_t));
    
    for (i=0;i<n;i++)
    {
        smm_players[i].pos = 0;
        smm_players[i].credit = 0;
        smm_players[i].energy = initEnergy;
        smm_players[i].flag_graduated = 0;
        
        printf("Input %i-th player name:", i);
        scanf("%s", &smm_players[i].name[0]); 
        fflush(stdin);
    }
 }



int rolldie(int player)
{
    char c;
    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);
 
    
#if 0
    if (c == 'g')
        printGrades(player);
#endif
    
    return (rand()%MAX_DIE + 1);
}



//action code when a player stays at a node
void actionNode(int player)
{
     void *ptr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
     
     
     int type = smmObj_getObjectType(ptr);
     int credit = smmObj_getObjectCredit(smm_players[player].pos);
     int energy = smmObj_getObjectEnergy(ptr);
     int grade;
     void *gradePtr;
     
     
     printf(" --> player%i pos : %i, type : %s, credit : %i, energy : %i\n",
               player, smm_players[player].pos, smmObj_getTypeName(type), credit, energy);
     
     
     switch(type)
     {
                 case SMMNODE_TYPE_LECTURE:
                 printf("Here is Lacture.\n"); 
                 if (findGrade(player, smmObj_getObjectName(ptr)) == NULL )
                 {
                      smm_players[player].credit += credit;
                      smm_players[player].energy -= energy;
                      
                      grade = rand()%SMMNODE_MAX_GRADE;
                      
                      gradePtr = smmObj_genObject(smmObj_getObjectName(ptr), SMMNODE_OBJTYPE_GRADE, type, credit, energy, grade);
                      
                      smmdb_addTail(LISTNO_OFFSET_GRADE+player, gradePtr);
                 }     
                      
                      break;
                      
                 case SMMNODE_TYPE_RESTAURANT:
                 printf("Here is Restaurant.\n"); 
                 smm_players[player].energy += energy;
                      break;
                      
                 case SMMNODE_TYPE_LABORATORY:
                 printf("Here is Laboratory.\n"); 
                 if (smmObj_getObjectType(ptr) == 4)
                 {
                      int ReferenceValue = 6;
                      int value = rolldie(player);
                      
                      printf("Die result is : %i\n", value);
                      
                      if (value == ReferenceValue)
                      {
                          smmObj_getObjectType(ptr,2);
                          printf("Escape!");
                      }
                      
                      else
                      {
                          smmObj_getObjectType(ptr,4);
                          printf("More More Experiment!");
                      }
                 }
                      break;
                      
                 case SMMNODE_TYPE_HOME:
                 printf("Here is Home.\n"); 
                      smm_players[player].energy += energy;
                      if (smm_players[player].credit >= GRADUATE_CREDIT)
                      {
                                                     smm_players[player].flag_graduated = 1;
                       }
                      break;
                      
                 case SMMNODE_TYPE_GOTOLAB:
                 printf("Here is Gotolab.\n"); 
                 if (smmObj_getObjectType(ptr) == 4)
                 {            
                      
                 }
                      break;
                      
                 case SMMNODE_TYPE_FOODCHANGE:
                 printf("Here is Foodchange.\n"); 
                 {
                      int food = rand()%smm_food_nr;
                              
                      void *foodPtr = smmObj_genObject(smmObj_getObjectName(ptr), SMMNODE_OBJTYPE_FOOD, type, credit, energy, grade);
                              
                      smmdb_addTail(LISTNO_FOODCARD+player, foodPtr);
                 }
                      break;
                      
                 case SMMNODE_TYPE_FESTIVAL:
                 printf("Here is Festival.\n"); 
                 {      
                      char festival = rand()%smm_festival_nr;
                              
                      void *festivalPtr = smmObj_genObject(smmObj_getObjectName(ptr), SMMNODE_OBJTYPE_FEST, type, credit, energy, grade);
                      
                      printf("Do your Mission! : ");
                              
                      smmdb_addTail(LISTNO_FESTCARD+player, festivalPtr);
                 }
                      break;
                      
        //case lecture:
        default:
            break;
    }
}



int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int turn;
    
    smm_board_nr = 0;
    smm_food_nr = 0;
    smm_festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while ( fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        void* ptr;
        printf("%s %i %i %i\n", name, type, credit, energy);
        ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_BOARD, type, credit, energy, 0);
        smm_board_nr = smmdb_addTail(LISTNO_NODE, ptr);   
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", smm_board_nr);
    
    
    
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i ", name, &energy) == 2) //read a food parameter set
    {
        //store the parameter set
        void* ptr;
        printf("%s %i \n", name, energy);
        ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_BOARD, type, credit, energy, 0);
        smm_food_nr = smmdb_addTail(LISTNO_NODE, ptr);
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", smm_food_nr);
    
    

    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s ", name) == 1) //read a festival card string
    {
        //store the parameter set
        void* ptr;
        printf("%s \n", name);
        ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_BOARD, type, credit, energy, 0);
        smm_festival_nr = smmdb_addTail(LISTNO_NODE, ptr);
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", smm_festival_nr);
    

    
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
        printf("Input player number: ");
        scanf("%i", &smm_player_nr);
        fflush(stdin);
        
        if (smm_player_nr <= 0 || smm_player_nr > MAX_PLAYER)
           printf("Invalid player number!\n");
    }
    while (smm_player_nr <= 0 || smm_player_nr > MAX_PLAYER);
    
    
    
    generatePlayers(smm_player_nr,smmObj_getObjectEnergy(smmdb_getData(SMMNODE_OBJTYPE_BOARD,0)));
    


    turn = 0;
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (isGraduated() == 0) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);
        //pos = pos + 2;
        
		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        turn = (turn +1)%smm_player_nr;
    }
    
    free(smm_players);
    
    system("PAUSE");
    return 0;
}
