#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct node { 
    char *name;
    struct relazione_inentrata *inentrata; //struttura per relazioni in entrata
    struct relazione_inuscita *inuscita;  //struttura per relazioni in uscita
    struct node *next;
    struct node *prev;          
}entity;

typedef struct relazione_inentrata{ 
    char *nome_relazione;
    struct node *entity;
    struct relazione_inentrata *next_rel;
    struct relazione_inentrata *prev_rel;
    struct same_rel *people;
    int numrel;//posso così creare dei links per la report senza dover utilizzare altre strutture dati aggiuntive
    struct relazione_inentrata *next_entity;
    struct relazione_inentrata *prev_entity;
}entrata;

typedef struct relazione_inuscita{
    char *nome_relazione;
    struct relazione_inuscita *next_rel;
    struct relazione_inuscita *prev_rel;
    struct same_rel *people;
}uscita;

typedef struct same_rel{//lista delle persone con cui ho lo stesso di tipo di relazione
    struct node *person;
    struct same_rel *p;
    struct same_rel *right;
    struct same_rel *left;
}stessa;

typedef struct relationtype{ //nodo che sarà la radice dell'albero per ogni tipo di relazione
    char *nome_relazione;
    struct relationtype *next_rel;//ordino le relazioni in ordine alfabetico per stamparle con la report.
    struct relationtype *prev_rel;
    struct relazione_inentrata *list_head; //puto direttamente al nodo nella struttura person in modo da creare una catena senza però creare altre strutture
    struct relazione_inentrata *list_queue;
}label;

//____________________________________________________FUNZIONI DI SERVIZIO_____________________________________________________________
struct relationtype * relationshipforest;

struct node *personhashtable[128];
void inizializzatabella1(struct node * personhashtable[]){
    for(int i=0; i<128; i++) {
        personhashtable[i]=NULL;        
    }
}

int personhashfunct(char name[]){ 
    int sum=0;
    for(int i=0; i<strlen(name); i++){
        sum=sum + (name[i]*i) ; 
    }
    sum=sum%128;
    return sum;
}

struct node *hashtablecheck(char name[], int sum){
    
    if (personhashtable[sum]==NULL)   return NULL;

    struct node *temp;
    temp=personhashtable[sum];
    int trovato=0;

    while(temp->next!=NULL && trovato==0){
        if(strcmp(temp->name, name)==0) trovato=1;
        else temp=temp->next;
    }

    if(strcmp(temp->name, name)==0) trovato=1;

    if(trovato==1) return temp;
    return NULL;        
    
} 

int samerel_listinsert(struct same_rel **list_head, struct node *ente){

    struct same_rel *x, *y;
    x=*list_head;
    y=x->p;
    
    while(x!=NULL){
        y=x;
        if(x->person==ente) return 0;
        if(strcmp(ente->name, x->person->name)<0) x=x->left;
        else x=x->right;
    }

    struct same_rel *z;
    z=(struct same_rel *)calloc(1, sizeof(struct same_rel));
    z->person=ente;

    if(strcmp(z->person->name, y->person->name)<0) y->left=z;
    else y->right=z;

    z->p=y;
    return 1;
}

void transplant (struct same_rel **list_head, struct same_rel **a, struct same_rel **b){
    struct same_rel *x, *y;
    x=*a;
    y=*b;
    
    if(x->p==NULL) *list_head=y;
    else if (x==x->p->left) x->p->left=y;
    else x->p->right=y;

    if(y!=NULL) y->p=x->p;
    return;
}

int samerel_listremove(struct same_rel **list_head, struct node *ente){

    struct same_rel *x, *y;
    x=*list_head;


    while(x!=NULL){
        if(strcmp(ente->name, x->person->name)==0) break;
        if(strcmp(ente->name, x->person->name)<0)x=x->left;
        else if(strcmp(ente->name, x->person->name)>0) x=x->right;
    }
    
    if(x==NULL) return 0;

    if(x->left==NULL)   transplant(&*list_head, &x, &x->right);
    else if(x->right==NULL) transplant(&*list_head, &x, &x->left);

    else {
        y=x->right;
        while(y->left!=NULL){
            y=y->left;
        }
        if(y->p!=x){
            transplant (&*list_head, &y, &y->right);
            y->right=x->right;
            y->right->p=y;
            y->p=x;
        }
        transplant(&*list_head, &x, &y);
        y->left=x->left;
        y->left->p=y;
    }   
    free(x);
    return 1;
}

void forest_listinsert(struct relazione_inentrata **list_head, struct relazione_inentrata **list_queue, struct relazione_inentrata **node){
    struct relazione_inentrata *x, *y;       

    if(*list_head==NULL){
       *list_head=*node;
       *list_queue=*node;    
    }

    else{
        x=*list_queue;
        y=*node;

        while(x!=NULL && strcmp(y->entity->name, x->entity->name)<0 && x->numrel==1){
            x=x->prev_entity;
        }

        if(x==NULL){
            y->next_entity=*list_head;
            *list_head=y;
            y->next_entity->prev_entity=y;
            y->prev_entity=NULL;
        }

        else if(strcmp(y->entity->name, x->entity->name)>0 || x->numrel>1){
            y->next_entity=x->next_entity;
            x->next_entity=y;
            if(y->next_entity==NULL) *list_queue=y;
            else y->next_entity->prev_entity=y;
            y->prev_entity=x;            
        }
    }  
}

void forest_listfixup(struct relazione_inentrata **list_head, struct relazione_inentrata **list_queue,  struct relazione_inentrata **node){ 

    struct relazione_inentrata *x, *y;
    x=*node;
    y=x->prev_entity;

    if(x==*list_head) return;    
    if(y->numrel > x->numrel)   return;
    if(y->numrel==x->numrel &&  strcmp(y->entity->name, x->entity->name)<0 ) return;

    //scorro la lista per trovare la nuova posizione del nodo
    while(y!=NULL && (x->numrel > y->numrel)){
        y=y->prev_entity;
    }
    while(y!=NULL && (x->numrel == y->numrel) && strcmp(x->entity->name, y->entity->name)<0){
    y=y->prev_entity;
    } 
    
    //creo i collegamenti necessari prima dello spostamento
    x->prev_entity->next_entity=x->next_entity; //non ho bisogno di controllare se prev_entity != NULL poiché il nodo sarebbe list_head e quindi return della funzione
    if(x->next_entity!=NULL) x->next_entity->prev_entity=x->prev_entity;
    else *list_queue=x->prev_entity;

    if(y==NULL){
        x->next_entity=*list_head;
        *list_head=x;
        x->next_entity->prev_entity=x;
        x->prev_entity=NULL;
    }

    else if((y->numrel > x->numrel) || (y->numrel==x->numrel && strcmp(y->entity->name, x->entity->name)<0)){

        x->next_entity=y->next_entity;
        x->next_entity->prev_entity=x;//non ho  bisogno di controllare se x->next != NULL poiché mi sposto sempre verso sinistra

        y->next_entity=x;
        x->prev_entity=y;
    }    
}

void forest_delete_listfixup(struct relazione_inentrata **list_head, struct relazione_inentrata **list_queue,  struct relazione_inentrata **node){

    struct relazione_inentrata *x, *y;
    x=*node;
    y=x->next_entity;

    if(x->numrel==0){
        if(x==*list_queue) *list_queue=x->prev_entity;
        else x->next_entity->prev_entity=x->prev_entity;

        if(x->prev_entity!=NULL) x->prev_entity->next_entity=x->next_entity;
        else *list_head=x->next_entity;

        return;
    }

    else if(x==*list_queue && x->numrel !=0) return;
    else if(x->numrel>y->numrel)return;
    else if((x->numrel==y->numrel) && strcmp(x->entity->name, y->entity->name)<0)return;
    
    //creo i collegamenti necessari prima dello spostamento
    if(x->prev_entity!=NULL)x->prev_entity->next_entity=x->next_entity;
    else *list_head=x->next_entity;
    x->next_entity->prev_entity=x->prev_entity; //non ho bisogno di controllare se x->next !=NULL poiché se così fosse sarebbe coda e quindi ho la return
    
    while(y!=NULL && (x->numrel < y->numrel)){
        y=y->next_entity;
    }
    while(y!=NULL && (x->numrel == y->numrel) && strcmp(x->entity->name, y->entity->name)>0){
    y=y->next_entity;
    }   

    if(y==NULL){
        x->prev_entity=*list_queue;
        *list_queue=x;

        x->prev_entity->next_entity=x;
        x->next_entity=NULL;
    }

    else if((x->numrel>y->numrel) || (x->numrel==y->numrel && strcmp(y->entity->name, x->entity->name)>0)){
        x->prev_entity=y->prev_entity;        
        x->prev_entity->next_entity=x;

        y->prev_entity=x;
        x->next_entity=y;
    }
}

//_____________________________________________________ FUNCTIONS_____________________________________________________

void add_ent(char name[], int j){

    if(personhashtable[j]==NULL){
        personhashtable[j]=(struct node *)calloc(1, sizeof( struct node));
        personhashtable[j]->name=(char *)calloc(strlen(name)+1,sizeof(char));
        strcpy(personhashtable[j]->name, name);
        }
        
    else {
        struct node *x;
        x=personhashtable[j];
        while(x!=NULL && strcmp(x->name, name)<0){
            if(strcmp(x->name, name)==0) return;   
            if(x->next==NULL) break;                    
            x=x->next;
        }

        if(strcmp(x->name, name)==0) return;   

        struct node *newent;
        newent=(struct node *)calloc(1, sizeof(struct node));
        newent->name=(char *)calloc(strlen(name)+1, sizeof(char));
        strcpy(newent->name, name);

        if(strcmp(x->name, name)>0){
            newent->prev=x->prev;
            if(newent->prev!=NULL)newent->prev->next=newent;
            else personhashtable[j]=newent;

            newent->next=x;
            x->prev=newent;            
        }

        else if(x->next==NULL) {
            x->next=newent;
            newent->prev=x;
        }
    }
}

void add_rel(struct node *ente1, struct node *ente2, char relazione[]){ 

    if(ente1==NULL || ente2==NULL) return;

    //_____________________ENTE1____________________________________________________________
    if(ente1->inuscita==NULL){
        ente1->inuscita=(struct relazione_inuscita *)calloc(1, sizeof(struct relazione_inuscita));
        ente1->inuscita->nome_relazione=(char *)calloc(strlen(relazione)+1, sizeof(char));
        strcpy(ente1->inuscita->nome_relazione, relazione);
        ente1->inuscita->people=(struct same_rel*)calloc(1, sizeof(struct same_rel));
        ente1->inuscita->people->person=ente2;
    }

    else{
        struct relazione_inuscita *temp1;
        temp1=ente1->inuscita;
        while(strcmp(temp1->nome_relazione, relazione)<0){            
            if(temp1->next_rel == NULL) break;
            temp1=temp1->next_rel; 
        }   

        if(strcmp(temp1->nome_relazione, relazione)==0){
            if(samerel_listinsert(&(temp1->people), ente2)==0)return;
        }

        else{
            struct relazione_inuscita *newrel;
            newrel=(struct relazione_inuscita *)calloc(1, sizeof(struct relazione_inuscita));
            newrel->nome_relazione=(char*)calloc(strlen(relazione)+1, sizeof(char));
            strcpy(newrel->nome_relazione, relazione);
            newrel->people=(struct same_rel*)calloc(1, sizeof(struct same_rel));
            newrel->people->person=ente2;

            if(strcmp(temp1->nome_relazione, relazione)>0){
                if(temp1->prev_rel!=NULL)temp1->prev_rel->next_rel=newrel;
                else ente1->inuscita=newrel;
                newrel->prev_rel=temp1->prev_rel;
                temp1->prev_rel=newrel;
                newrel->next_rel=temp1;               
            }
            else{
                temp1->next_rel=newrel;
                newrel->prev_rel=temp1;
            }
        }               
    }

    //_____________________________________________________ENTE2______________________________________________________
    struct relazione_inentrata *node;//nodo di ente 2 per salvarmi il collegamento nella forest

    if(ente2->inentrata==NULL){
        ente2->inentrata=(struct relazione_inentrata *)calloc(1, sizeof(struct relazione_inentrata));
        ente2->inentrata->nome_relazione=(char *)calloc(strlen(relazione)+1, sizeof(char));
        strcpy(ente2->inentrata->nome_relazione, relazione);
        ente2->inentrata->people=(struct same_rel*)calloc(1, sizeof(struct same_rel));
        ente2->inentrata->people->person=ente1;
        ente2->inentrata->numrel=1;
        ente2->inentrata->entity=ente2;
        node=ente2->inentrata;
        //devo ancora inserire il nuovo nodo creato all'interno della struttura fittizia delle relazioni
    }

    else{
        struct relazione_inentrata *temp2;
        temp2=ente2->inentrata;
        while(strcmp(temp2->nome_relazione, relazione)<0){            
            if(temp2->next_rel == NULL) break;
            temp2=temp2->next_rel;       
        }   

        if(strcmp(temp2->nome_relazione, relazione)==0){
            samerel_listinsert(&temp2->people, ente1);  //devvo aggiungere la persona che mi fa la relazione in ingresso!
            temp2->numrel+=1;
            node=temp2; //successivamente chiamerò la fixup            
        }

        else{//se non ho ancora questo tipo di relazione in entrata
            struct relazione_inentrata *newrel;
            newrel=(struct relazione_inentrata *)calloc(1, sizeof(struct relazione_inentrata));
            newrel->nome_relazione=(char*)calloc(strlen(relazione)+1, sizeof(char));
            strcpy(newrel->nome_relazione, relazione);
            newrel->people=(struct same_rel*)calloc(1, sizeof(struct same_rel));
            newrel->people->person=ente1;
            newrel->numrel=1;
            newrel->entity=ente2;

            if(strcmp(temp2->nome_relazione, relazione)>0){
                if(temp2->prev_rel!=NULL)temp2->prev_rel->next_rel=newrel;
                else ente2->inentrata=newrel;
                newrel->prev_rel=temp2->prev_rel;
                temp2->prev_rel=newrel;
                newrel->next_rel=temp2;               
            }
            else{
                newrel->next_rel=temp2->next_rel;
                if(newrel->next_rel!=NULL)newrel->next_rel->prev_rel=newrel;
                temp2->next_rel=newrel;
                newrel->prev_rel=temp2;
            }           
            node=newrel; //devo chiamare la list insert per aggiornare la struttura relazioni e inserire il nodo tra i vari link;
        }       
    }    
    
    //____________________________________AGGIORNO LA STRUTTURA GENERALE DELLE RELAZIONI____________________________________    
    struct relationtype *list;   
    if(relationshipforest==NULL){
        relationshipforest=(struct relationtype *)calloc(1,sizeof(struct relationtype));
        relationshipforest->nome_relazione=(char*)calloc(strlen(relazione)+1, sizeof(char));
        strcpy(relationshipforest->nome_relazione, relazione);
        list=relationshipforest; 
    }

    else{
        list=relationshipforest; 
        while(strcmp(list->nome_relazione, relazione)<0) {
            if(list->next_rel==NULL) break;
            list=list->next_rel;
             //se trova list allora strcmp=0 per tanto list si troverà sulla giusta posizione poiché il while termina;
        }

        if(strcmp(list->nome_relazione, relazione)!=0){
            struct relationtype *newnode;
            newnode=(struct relationtype *)calloc(1,sizeof(struct relationtype));
            newnode->nome_relazione=(char*)calloc(strlen(relazione)+1, sizeof(char));
            strcpy(newnode->nome_relazione, relazione);

            if(strcmp(list->nome_relazione, newnode->nome_relazione)>0){            
                newnode->next_rel=list;
                newnode->prev_rel=list->prev_rel;
                list->prev_rel =newnode;
                if(newnode->prev_rel==NULL) relationshipforest=newnode;
                else newnode->prev_rel->next_rel=newnode;
                list=newnode;
            }
            else if(list->next_rel==NULL) {
                list->next_rel=newnode;
                newnode->prev_rel=list;
                list=list->next_rel;
            }
        }       
    }
 
    if(node->numrel==1)forest_listinsert(&list->list_head, &list->list_queue, &node);

    else forest_listfixup(&list->list_head, &list->list_queue, &node);   
    
}

void del_rel(struct node **ente_one, struct node **ente_two, char *relazione){

    struct node *ente1, *ente2;
    ente1=*ente_one;
    ente2=*ente_two;

    if(ente1==NULL || ente2==NULL || ente1->inuscita==NULL || ente2->inentrata==NULL) return;

    struct relazione_inuscita *temp1;
    temp1=ente1->inuscita;

    while(strcmp(temp1->nome_relazione, relazione)!=0){
        if(temp1->next_rel==NULL && strcmp(temp1->nome_relazione, relazione)!=0 )return;
        temp1=temp1->next_rel;
    }

   if (samerel_listremove(&temp1->people, ente2)==0) return;

   if(temp1->people==NULL){
       if(temp1->prev_rel==NULL)ente1->inuscita=temp1->next_rel;
       else temp1->prev_rel->next_rel=temp1->next_rel;

       if(temp1->next_rel!=NULL)temp1->next_rel->prev_rel=temp1->prev_rel;

       free(temp1->people);       
       free(temp1->nome_relazione);
       free (temp1);
   }

   struct relazione_inentrata *temp2;
   temp2=ente2->inentrata;

    while(strcmp(temp2->nome_relazione, relazione)!=0){
        if(temp2->next_rel==NULL && strcmp(temp2->nome_relazione, relazione)!=0 )return;
        temp2=temp2->next_rel;
    }

    samerel_listremove(&temp2->people, ente1);
    temp2->numrel=temp2->numrel -1;

    struct relationtype *temp3;
    temp3=relationshipforest;

    while(strcmp(temp3->nome_relazione, relazione)!=0){
        temp3=temp3->next_rel;
    }

    forest_delete_listfixup(&temp3->list_head, &temp3->list_queue, &temp2);
    //ho sistemato la relation_ship forest: ora posso eventualmente eliminare il nodo dalle relazini in entrata se numrel è diventato = 0;
    if (temp2->numrel==0){
        if(ente2->inentrata==temp2) ente2->inentrata=temp2->next_rel;
        else temp2->prev_rel->next_rel=temp2->next_rel;

        if(temp2->next_rel!=NULL)temp2->next_rel->prev_rel=temp2->prev_rel;

        free(temp2->people);
        free(temp2->nome_relazione);
        free(temp2);
    }

    if(temp3->list_head==NULL){
        if(temp3->prev_rel==NULL) relationshipforest=temp3->next_rel;
        else temp3->prev_rel->next_rel=temp3->next_rel;

        if(temp3->next_rel!=NULL)temp3->next_rel->prev_rel=temp3->prev_rel;

        free(temp3->nome_relazione);
        free(temp3);        
    }
}

void del_ent(struct node *ente, int j){

    if(ente==NULL)  return;

    struct same_rel *temp;
    char relazione[50];
    
    while(ente->inuscita!=NULL){
        temp=ente->inuscita->people;
        strcpy(relazione, ente->inuscita->nome_relazione);
        while(ente->inuscita->people!=NULL){
            del_rel(&ente, &temp->person, relazione);
            if(ente->inuscita!=NULL) temp=ente->inuscita->people;
            else break;
            if(strcmp(relazione, ente->inuscita->nome_relazione)!=0) break;
        }        

    }

    while (ente->inentrata!=NULL){
        temp=ente->inentrata->people;
        strcpy(relazione, ente->inentrata->nome_relazione);
        while(ente->inentrata->people!=NULL){
            del_rel(&temp->person, &ente, relazione);
            if(ente->inentrata!=NULL) temp=ente->inentrata->people;
            else break;
            if(strcmp(relazione, ente->inentrata->nome_relazione)!=0) break;
        }        
    }

    if(personhashtable[j]==ente){
        personhashtable[j]=ente->next;
        if(personhashtable[j]!=NULL) personhashtable[j]->prev=NULL;
    } 
    else ente->prev->next=ente->next;
    if(ente->next!=NULL)ente->next->prev=ente->prev;    

    free(ente->name);
    free(ente);
    return;
}

void printmaxnodes(struct relazione_inentrata *list_head){
    struct relazione_inentrata *x;
    x=list_head;
    while(x!=NULL && x->numrel==list_head->numrel){
        fputs(x->entity->name, stdout);
        fputs(" ", stdout);
        x=x->next_entity;
    } 
    fprintf(stdout, "%d;",list_head->numrel);
    fputs(" ", stdout);
}

void report(){
    if(relationshipforest==NULL){
        fputs("none\n", stdout);
        return;
    }
    else{
        struct relationtype *label;
        label=relationshipforest;
        while(label->next_rel!=NULL){//sto separando l'ultimo elemento, altrimenti metterei lo spazio prima di andare a capo
            fputs(label->nome_relazione, stdout);
            fputs(" ", stdout);
            printmaxnodes(label->list_head);            
            label=label->next_rel;
        }
        fputs(label->nome_relazione, stdout);
        fputs(" ", stdout);
        struct relazione_inentrata *x;
        x=label->list_head;
        while(x!=NULL && x->numrel==label->list_head->numrel){
            fputs(x->entity->name, stdout);
            fputs(" ", stdout);
            x=x->next_entity;
        } 
        fprintf(stdout, "%d;", label->list_head->numrel);
        }
    fputs("\n", stdout);
}

int main(){

    inizializzatabella1(personhashtable);   
    relationshipforest=NULL;

    char to_do[4][50], c='\0';
    int i=0, j=0, k=0;

    while(1){ 

        while(c!=' ' && c!= '\n'){
           c=getc_unlocked(stdin);
           if(c!=' ' &&  c!='\n' && c!='\r')   to_do[0][k]=c;
           else to_do[0][k]='\0';
           k++;
       }
       k=0; c='\0';
       

        if(to_do[0][0]=='a' && to_do[0][3]=='e'){//________ADD_ENT_____________
            
            while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[1][k]=c;
                else to_do[1][k]='\0';
                k++;
            }
            k=0; c='\0';

            j=personhashfunct(to_do[1]);      
            add_ent(to_do[1], j);

        }

        else if(to_do[0][0]=='a' && to_do[0][3]=='r'){ //____________ADD_REL____________

            while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[1][k]=c;
                else to_do[1][k]='\0';
                k++;
            }
            k=0; c='\0';
            
            while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[2][k]=c;
                else to_do[2][k]='\0';
                k++;
            }
            k=0; c='\0';
            
            while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[3][k]=c;
                else to_do[3][k]='\0';
                k++;
            }
            k=0; c='\0'; 

            j=personhashfunct(to_do[1]);
            i=personhashfunct(to_do[2]);

            add_rel(hashtablecheck(to_do[1], j), hashtablecheck(to_do[2], i), to_do[3]);
        }
        
        else if(to_do[0][0]=='d' && to_do[0][3]=='e'){ //__________DEL_ENT_______________

            while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[1][k]=c;
                else to_do[1][k]='\0';
                k++;
            }
            k=0; c='\0';

            j=personhashfunct(to_do[1]);      
            del_ent(hashtablecheck(to_do[1], j), j);
        }
            
        else if (to_do[0][0]=='d' && to_do[0][3]=='r'){//___________DEL_REL________________

             while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[1][k]=c;
                else to_do[1][k]='\0';
                k++;
            }
            k=0; c='\0';
            
            while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[2][k]=c;
                else to_do[2][k]='\0';
                k++;
            }
            k=0; c='\0';
            
            while(c!=' ' && c!= '\n'){
                c=getc_unlocked(stdin);
                if(c!=' ' &&  c!='\n' && c!='\r')   to_do[3][k]=c;
                else to_do[3][k]='\0';
                k++;
            }
            k=0; c='\0'; 

            j=personhashfunct(to_do[1]);
            i=personhashfunct(to_do[2]);

            struct node *ente1, *ente2;
            ente1=hashtablecheck(to_do[1], j);
            ente2=hashtablecheck(to_do[2], i);

            del_rel(&ente1, &ente2, to_do[3]);            
        }

        else if(to_do[0][0]=='r') {//____REPORT_______
            report();
        }
        
        else if(to_do[0][0]=='e') {//________END________
            return 0;
        }
    }
}