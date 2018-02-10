#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "./dns_resource_records.h"

int main(){

    char *filename = "dns-master.txt";

    Resource_record *head  = (Resource_record *) malloc(sizeof(Resource_record));

    file_to_list(filename, head);
    
    Resource_record *search = search_record_names(head, "ns1.student.test");

    if(search){
        printf("location: %s\n", search->location);
    }

    return 0;
}
