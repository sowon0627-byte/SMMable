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


//function prototypes
void generatePlayers(int n, int initEnergy); //generate a new player
void printPlayerStatus(void); //print all player status at the beginning of each turn
void printGrades(int player); //print grade history of the player
void printAllGrades(int player); //print all the grade history of the player
float calcAverageGrade(int player); //calculate average grade of the player


float calcAverageGrade(int player) //calculate average grade of the player
{
    int i;
    int totalGrade = smmdb_len(LISTNO_OFFSET_GRADE + player);
    float sumGrade = 0.0f;
    
    float gradePoints[] = {4.3, 4.0, 3.7, 3.3, 3.0, 2.7, 2.3, 2.0, 1.7, 1.3, 1.0, 0.7, 0.0};
    
    for (i = 0; i < totalGrade; i++)
    {
        void* gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        if (gradePtr != NULL)
        {
            int grade = (int)smmObj_getObjectGrade(gradePtr);
            
            sumGrade += gradePoints[grade];
        }
    }

    return sumGrade / (float)totalGrade;
}




void printAllGrades(int player) //print all the grade history of the player
{
     int i;
     int totalGrade = smmdb_len(LISTNO_OFFSET_GRADE + player);
     
     for (i=0;i<totalGrade;i++)
     {
         void* SubjectPtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
         
         char* subjectName = smmObj_getObjectName(SubjectPtr);
         int subjectcredit = smmObj_getObjectCredit(SubjectPtr);
         int TotalGrade = smmObj_getObjectGrade(SubjectPtr);
         
         smmGrade_e tempType = (smmGrade_e)TotalGrade; 
         char* gradeStr = smmObj_getGradeName(tempType);
         
         printf("%s | %d | %s\n", subjectName, subjectcredit, gradeStr);
         
     }
     
     float avgGrade = calcAverageGrade(player);
     
     printf("Final Grade : %f\n", avgGrade);
}


void printGrades(int player) //print grade history of the player
{
     int i;
     int size = smmdb_len(LISTNO_OFFSET_GRADE + player);
     void* listPtr;
     void* GradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, 0);
     
     //if player haven't take any course
     if (GradePtr == NULL)
     {
                  printf("Haven't take any Course.\n");
     }
     
     else
     {
         for (i = 0; i <size; i++) 
         {
            listPtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
            char* lectureName = smmObj_getObjectName(listPtr);
            
            printf("%s\n", lectureName); 
          }
     }
     
     printf("Total Grade: %d.\n", smm_players[player].credit);
}
     


smmGrade_e takeLecture(int player, char *lectureName, int credit) //take the lecture (insert a grade of the player)
{
           smmGrade_e grade;
           
           int score = rand()%SMMNODE_MAX_GRADE;
           grade = (smmGrade_e)score;
           
           printf("Complete! %s course (%d credits) get!\n", lectureName, credit);
           printf("Grade: %s\n", smmObj_getGradeName(grade));
           
           return grade;
}



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
                        
                        if (smm_players[player].pos == 0)
                        {
                        
                            if (smm_players[player].credit >= GRADUATE_CREDIT)
                            {
                                                           smm_players[player].flag_graduated = 1;
                                                           printAllGrades(player);
                                                           printf("Congratulations! %s Graduate!", smm_players[player].name);
                                                           break;
                            }
                        }
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
 
    

    if (c == 'g')
        printGrades(player);

    
    return (rand()%MAX_DIE + 1);
}



//action code when a player stays at a node
void actionNode(int player)
{
     void *ptr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
     
     if(ptr == NULL)
     return;
     
     int type = smmObj_getObjectType(ptr);
     int credit = smmObj_getObjectCredit(ptr);
     int energy = smmObj_getObjectEnergy(ptr);
     int grade;
     void *gradePtr;
     
     
     printf(" --> player%i pos : %i, type : %s, credit : %i, energy : %i\n",
               player, smm_players[player].pos, smmObj_getTypeName(ptr), credit, energy);
     
     
     switch(type)
     {
                 case SMMNODE_TYPE_LECTURE:
                 printf("Here is Lacture.\n"); 
                 if (findGrade(player, smmObj_getObjectName(ptr)) == NULL )
                 {
                      if(smm_players[player].energy >= energy)
                      {
                          smm_players[player].credit += credit;
                          smm_players[player].energy -= energy;
                          
                          char* LectureName = smmObj_getObjectName(ptr);
                          
                          smmGrade_e myGrade = takeLecture(player, LectureName, credit);
                          
                          gradePtr = smmObj_genObject(smmObj_getObjectName(ptr), SMMNODE_OBJTYPE_GRADE, type, credit, energy, (int)myGrade);
                          smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
                      }
                      else
                      {
                          printf("You can't take this lecture!\n");
                      }
                      
                      
                 }
                 else
                 {
                     printf("You have already completed this course!\n");
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
                 {
                      printf("Here is Foodchange.\n"); 
                      printf("Press any keys.\n");
                      
                      int randomFoodCard = rand()% smm_food_nr;
                      
                      void* foodPtr = smmdb_getData(LISTNO_FOODCARD, randomFoodCard);
                      
                      char* foodName = smmObj_getObjectName(foodPtr);
                      int foodEnergy = smmObj_getObjectEnergy(foodPtr);
                      
                      smm_players[player].energy += foodEnergy;
                      
                      printf("You have %s! Get %d Energy!\n", foodName, foodEnergy); 
                 }
                      break;
                      
                 case SMMNODE_TYPE_FESTIVAL:
                 {
                      printf("Here is Festival.\n");
                      printf("Press any keys.\n");       
                      
                      int randomFestivalCard = rand()% smm_festival_nr;
                      
                      void* festivalPtr = smmdb_getData(LISTNO_FESTCARD, randomFestivalCard);
                      
                      char* festivalName = smmObj_getObjectName(festivalPtr);

                      printf("Do your Mission! %s : \n", festivalName);
                      char answer[100];
                      scanf("%s", answer);
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
        smm_board_nr++;
        smmdb_addTail(LISTNO_NODE, ptr);   
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", smm_board_nr);
    
    
    
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    while (fscanf(fp, "%s %i ", name, &energy) == 2) //read a food parameter set
    {
        //store the parameter set
        void* ptr;
        ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_BOARD, 0, 0, energy, 0);
        smm_food_nr++;
        smmdb_addTail(LISTNO_FOODCARD, ptr);
    }
    fclose(fp);
    
    

    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    while (fscanf(fp, "%s ", name) == 1) //read a festival card string
    {
        //store the parameter set
        void* ptr;
        ptr = smmObj_genObject(name, SMMNODE_OBJTYPE_BOARD, 0, 0, 0, 0);
        smm_festival_nr++;
        smmdb_addTail(LISTNO_FESTCARD, ptr);
    }
    fclose(fp);
    
    

    
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
        
		//4-4. take action at the destination node of the board
		if (smm_players[turn].flag_graduated == 0)
		{
              actionNode(turn);
        }
        
        //4-5. next turn
        turn = (turn +1)%smm_player_nr;
    }
    
    free(smm_players);
    
    system("PAUSE");
    return 0;
}
