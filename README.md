# toy-interpret
a toy interpret inspired by c4 that explicit generate ast and interpret.

## example
```
function fib(x){
    g = 0;
    while(g<=5){
        g = g + 1;
    }
    if(x <= 2){
        return 1;
    }
    return fib(x-1) + fib(x-2) + g;
}

x = fib(20);
print(x);
```

## usage: 
  - cd src/
  - g++ *.cpp -std=c++11 -g3
  - ./a.out example
