#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TAILLE 300000 
typedef int tableau[TAILLE];


void tri_insertion(tableau T);
void afficher(tableau T);
int aleatoires();
void initTab(tableau T);
int partition(tableau T,int debut,int fin,int pivot);
void triRapide(tableau T, int debut, int fin);


int main(){
    time_t t1 = time(NULL);
    time_t t2 = time(NULL);
    tableau T = {5,3,9,8,7,5,2,1,9,1};
    initTab(T);
    t1 = time(NULL);
    tri_insertion(T);
    t2 = time(NULL);
    afficher(T);
    printf("\n");
    printf("duree = %.3f secondes\n", difftime(t2, t1) );
    return EXIT_SUCCESS;
}


void tri_insertion(tableau T){
    int x , j ;
    for (int i = 1 ;  i <= TAILLE-1 ; i++)
    {
        x = T[i];
        j = i;
        while ( (j > 0) && (T[j-1]> x) ){
            T[j] = T[j-1];
            j-- ;
        }
        T[j] = x ;
    }
}

void afficher(tableau T){
    for ( int i = 0 ; i < TAILLE ; i++){
        printf("%d " ,T[i]);
    }

}

int aleatoires(){
    srand(time(NULL));
    int x = rand();
    return x ;
}


void initTab(tableau T){
    for (int i = 0 ;  i <= TAILLE ; i++){
        T[i] = aleatoires();
    }
}

int partition(tableau T,int debut,int fin,int pivot){
    int temp = T[fin]; 
    T[fin] = T[pivot];
    T[pivot] = temp ;
    int j = debut ;
    for ( int i = debut ; i <= (fin-1) ; i++ ){
        if (T[i] <= T[fin]){
            temp = T[i]; 
            T[i] = T[j];
            T[j] = temp ;
            j++ ;
        }
    }
    
    temp = T[fin]; 
    T[fin] = T[j];
    T[j] = temp ;
    return j ;
}

void triRapide(tableau T, int debut, int fin){
    int pivot ;
    if ( debut < fin ){
        pivot = (debut + fin)/2;
        pivot = partition(T, debut, fin, pivot);
        triRapide(T, debut, pivot-1);
        triRapide(T, pivot+1, fin);
    }
}

