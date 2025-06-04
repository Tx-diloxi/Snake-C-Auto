#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TAILLE 10
typedef int tableau[TAILLE];

int partition(tableau T,int debut,int fin,int pivot);
void triRapide(tableau T, int debut, int fin);
void afficher(tableau T);



int main(){
    tableau T = {5,3,9,8,7,5,2,1,9,1};
    triRapide(T, 0, TAILLE);
    afficher(T);
    printf("\n");
    return EXIT_SUCCESS;
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

void afficher(tableau T){
    for ( int i = 0 ; i < TAILLE ; i++){
        printf("%d " ,T[i]);
    }

}
