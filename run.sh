upcxx-run -shared-heap 1024M -n 16 $(upcxx-nodes nodes) ./out 
#valgrind --leak-check=full --track-origins=yes