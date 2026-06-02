#include <stdio.h>
#include <stddef.h>

struct some_data{

    char a;
    int b;
    char c;
    int d;

};

struct some_data data;

#define container_of(ptr , type , member) ((type*)((char*)ptr - (long)(&((type*)0)->member)) )

#define container_of_customised(ptr , type , member) ((type*)((char *)ptr - offsetof(type , member)))

void get_container(char *ptr){

    struct some_data temp;

    long offset;

    // int offset = (int)((char*)&temp.c - (char*)&temp.a);
    //Another way of calculating the above offset

    // offset = (long)(&((struct some_data*)0)->c);

    // struct some_data *data1 = (struct some_data*)(ptr - offset);
//    struct some_data *data1 =  container_of(ptr , struct some_data , c);

    struct some_data *data1 =  container_of_customised(ptr , struct some_data , c);


    if(&data == data1){
        printf("Data pointer value [ %p ]\n" , &data);
        printf("Derived pointer value [ %p ]\n" , data1);
    }

}


int main(){

    data.a = 10;
    data.b = 5;
    data.c = 'a';
    data.d = 100;

    get_container(&data.c);

    return 0;
}