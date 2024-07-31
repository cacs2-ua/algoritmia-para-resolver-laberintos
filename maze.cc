#include <iostream> 
#include <limits>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream> 
#include <utility>
#include <queue>

//MODE 1: QUEUE
//ELSE: MODE IS PRIORITY QUEUE

#define Q 1
#define PQ 2
#define MODE PQ

constexpr int SENTINEL = -1;


std::vector<std::pair<int,int>> camino;
struct maze_t {
    std::vector<std::vector<int>> maze;
    int n = 0;
    int m = 0;
    int min = 0;
    bool path2D = false;
    bool path = false;
    std::vector<std::vector<std::pair<int,int>>> camino;

    std::vector<std::vector<int>> to;
    std::vector<std::vector<int>> from;
    
};

int mejorCamino = std::numeric_limits<int>::max();


int maze_it_matrix(maze_t& maze, int x, int y);

int maze_it_matrix(const std::vector<std::vector<int>>& maze, int n, int m, std::vector<std::vector<int>>& resultados, int x, int y);

int visit = 0, explored = 0, leaf = 0, unfeasible = 0, notPromising = 0, promisingButDiscarded = 0, updatedFromPesimisticBounds = 0, updatedFromLeafs = 0;

void maze_bb(const std::vector<std::vector<int>>&maze, int n, int m, 
std::vector<std::vector<int>>&to,
std::vector<std::vector<int>>&from,
std::vector<std::vector<std::pair<int,int>>>&camino

){

    mejorCamino = from[1][1];

    using Position = std::tuple<int, int, int, int>; //cota optimista, x, y, distanciaRecorrida

    if (maze[0][0] == 0) return;

        struct isWorse{
            bool operator() (const Position& a, const Position& b){
                return std::get<0>(a)+std::get<3>(a) > std::get<0>(b)+std::get<3>(b);
            }
        };

    #if MODE==Q
    std::queue<Position> nodosVivos;
    #else
    std::priority_queue<Position, std::vector<Position>,isWorse> nodosVivos;
    #endif

    nodosVivos.emplace(std::max(n,m),0,0, 1 /*,m+n*/);
    while (!nodosVivos.empty()){
        int min, x, y, distanciaRecorrida; 
        std::tie(min, x, y, distanciaRecorrida) = 
        #if MODE==Q
        nodosVivos.front();  
        #else
        nodosVivos.top();  
        #endif
        nodosVivos.pop();


        if(x == n-1 && y == m-1 && maze[x][y]) {
            //min = distanciaRecorrida;     
            leaf++;

            if (distanciaRecorrida < mejorCamino) {mejorCamino = distanciaRecorrida; updatedFromLeafs++;}
            #if MODE == Q
            return; 
            #endif
        }
        if(x<0 || y<0) continue;

        if(!maze[x][y]) {unfeasible++;continue;}
        
        for(int i = 1; i>-2 ; i--){
            for (int j = 1;j>-2;j--){

                if(!i  && !j ) continue; //SI ES LA MISMA POSICION

                if( x+i <0 || y+j <0 || x+i>=n || y+j >=m) {unfeasible++;continue;} //SI SE SALE DE LA MATRIZ
        
                if(!maze[x+i][y+j])  {unfeasible++;continue;}//SI ES PARED

                visit++;

                if(from[x+i+1][y+j+1] != std::numeric_limits<int>::max() and from[x+i+1][y+j+1] + distanciaRecorrida > mejorCamino) {promisingButDiscarded++;continue;}
                
                if(from[x+i+1][y+j+1] != std::numeric_limits<int>::max()) {mejorCamino = from[x+i+1][y+j+1] + distanciaRecorrida + 1; updatedFromPesimisticBounds++; }

                if(to[x+i+1][y+j+1] <= distanciaRecorrida + 1) {notPromising++; continue; } //SI HA LLEGADO ANTES CON UNA DISTANCIA MENOR
                
                if(distanciaRecorrida + min + 1 > mejorCamino) {notPromising++;continue;} //DISTANCIA CHEBYCHEV

                to[x+i+1][y+j+1] = distanciaRecorrida+1; 
                
                camino[x+i][y+j] = {x,y};
                
                explored++;

                nodosVivos.emplace(std::max(n - (x+i), m - (y + j)), x+i, y+j, distanciaRecorrida+1/*n - (x+i) +  m - (y + j)*/);
            }
        }
    }


}


int maze_bb(maze_t &maze){
    maze_bb(maze.maze, /*x, y, decisiones,*/ maze.n, maze.m,/* maze.min, */maze.to, maze.from, maze.camino);/*mejorCamino);*/
    return mejorCamino;
}

int maze_it_matrix_inverso(const std::vector<std::vector<int>>& maze, int n, int m, std::vector<std::vector<int>>& resultados) {

    for (int i = n; i > 0; i--) {

        for (int j = m; j > 0; j--) {
            if (maze[i-1][j-1]) {
                
                resultados[i][j] = std::min({
                    j+1 > m or resultados[i][j+1] == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : resultados[i][j+1] + 1,
                    i+1 > n or resultados[i+1][j] == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : resultados[i+1][j] + 1,
                    i+1 > n or j+1 > m or resultados[i+1][j+1] == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : resultados[i+1][j+1] + 1
                    });

                if (i == n and j == m) resultados[i][j] = maze[i-1][j-1] == 0 ? std::numeric_limits<int>::max() : 1;


            }
            else resultados[i][j] = std::numeric_limits<int>::max();
        }
    }

    return resultados[1][1];
    
}

int maze_it_matrix(const std::vector<std::vector<int>>& maze, int n, int m, std::vector<std::vector<int>>& resultados, int x, int y) {

    for (int i = 0; i < n; i++) {

        for (int j = 0; j < m; j++) {
            if (maze[i][j]) {
                
                resultados[i+1][j+1] = std::min({
                    resultados[i][j+1] == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : resultados[i][j+1] + 1,
                    resultados[i+1][j] == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : resultados[i+1][j] + 1,
                    resultados[i][j] == std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : resultados[i][j] + 1

                    });

                if (i == x and j == y) resultados[i+1][j+1] = maze[x][y] == 0 ? std::numeric_limits<int>::max() : 1;


            }
            else resultados[i+1][j+1] = std::numeric_limits<int>::max();
        }
    }

    return resultados[n][m];
    
}

int maze_it_matrix(maze_t& maze, int x, int y) {
/*
    std::vector<std::vector<int>> resultados(maze.n + 1, std::vector<int>(maze.m + 1, std::numeric_limits<int>::max()));
    maze.tablaIterativa = resultados;
    */

    return maze_it_matrix(maze.maze, maze.n, maze.m, maze.to,x,y);

}

int maze_it_matrix_inverso(maze_t& maze) {


    return maze_it_matrix_inverso(maze.maze, maze.n, maze.m, maze.from);

}

maze_t argumentManagement(int argc, char* argv[]) {
    
    maze_t maze;

    if(argc == 1){
        std::cerr << "Usage:"<< std::endl;
        std::cerr << "maze [-p] [--p2D] -f file" << std::endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc;i++) {

        std::string s = argv[i];

        if (s == "-p") maze.path = true;

        else if (s == "-f") {

            if (++i > argc-1) {
                std::cerr << "ERROR: missing filename."<<std::endl;
                exit(EXIT_FAILURE);
            }

            std::string fileName  {argv[i]};

            std::ifstream is{fileName};
            if (!is) {
                std::cerr << "ERROR: canâ€™t open file: " << fileName << std::endl;
                exit(EXIT_FAILURE);
            }

            is >> maze.n >> maze.m;

            maze.maze = std::vector <std::vector<int>>(maze.n, std::vector<int>(maze.m));
            
            std::stringstream buffer;

            buffer << is.rdbuf();

            std::string mazeStr = buffer.str();

            int k = 0;
            char *endPtr;
            for (int i = 0; i < maze.n; i++){
                for (int j = 0; j < maze.m; j++,k++){     
                endPtr = &mazeStr[k];
                while (*endPtr != '1' and *endPtr != '0'){ endPtr = &mazeStr[k];k++;}
                maze.maze[i][j] = *endPtr=='1' ? 1 : 0;
                } 
            }
        }


        else if (s == "--p2D") maze.path2D = true;

        else {
            std::cerr << "ERROR: unknown option " << s<<std::endl;
            std::cerr << "Usage:"<< std::endl;
            std::cerr << "maze [-p] [--p2D] -f file" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
   
    return maze;
}

std::string maze_parser(maze_t&maze){

    if(maze.maze[maze.n-1][maze.m-1]) maze.maze[maze.n-1][maze.m-1] = 2;
    //if(maze.maze[0][0]) maze.maze[0][0] = 2;

    int x, y;
    std::tie(x,y) = maze.camino[maze.n-1][maze.m-1];

    std::cout<<maze.camino[maze.n-1][maze.m-1].first<<" "<<maze.camino[maze.n-1][maze.m-1].second<<std::endl;
    while(x!= -1 and y != -1){
        maze.maze[x][y] = 2;
        std::tie(x,y) = maze.camino[x][y];
    }
    

        std::string s = "";
    for(int i = 0; i<maze.n;i++){
        for(int j = 0; j<maze.m; j++){
            if(maze.maze[i][j] == 0) s+='0';
            else if(maze.maze[i][j] == 1) s+='1';
            else if (maze.maze[i][j] == 2) s+='*';

        }
        s+='\n';
    }
    return s;
}

std::string maze_parser1D(maze_t&maze){
  
    int n = maze.n-1, m = maze.m-1;

    int x, y;

    std::tie(x,y) = maze.camino[maze.n-1][maze.m-1];

    std::string s = "\n>";

    while(x != -1 and y != -1){

        if(n-x == 0 and m-y == -1)//oeste
            s+="7";
        if(n-x == 0 and m-y == 1)//este
            s+="3";
        if(n-x == 1 and m-y == 1)//sureste
            s+="4";
        if(n-x == 1 and m-y == 0)//sur
            s+="5";
        if(n-x == 1 and m-y == -1)//suroeste
            s+="6";
        if(n-x == -1 and m-y == 1)//noreste
            s+="2";
        if(n-x == -1 and m-y == 0)//norte
            s+="1";
        if(n-x == -1 and m-y == -1)//noroeste
            s+="8";
        n = x, m = y;
        std::tie(x,y) = maze.camino[x][y];
    }
    s+="<";
    std::reverse(s.begin(),s.end());
    return s;
}

int main(int argc, char* argv[]){

    maze_t maze = argumentManagement(argc, argv);

    double start = clock(); 

    std::vector<std::vector<std::pair<int,int>>> camino(maze.n, std::vector<std::pair<int,int>>(maze.m,{-1,-1}));

    std::vector<std::vector<int>> resultados(maze.n + 1, std::vector<int>(maze.m + 1, std::numeric_limits<int>::max()));

    maze.camino = camino;
    maze.to = resultados;
    maze.from = resultados;

    maze_it_matrix(maze,0,0); //calcular matriz desde 00 a nm

    maze_it_matrix_inverso(maze); //matriz desde nm a 00

    for(size_t i = 1;i<maze.to.size();i++){
        for(size_t j = 1; j<maze.to[0].size();j++){
            maze.to[i][j] = maze.to[i][j] == std::numeric_limits<int>::max() ?  maze.to[i][j] :  maze.to[i][j]+1;
            maze.from[i][j] = maze.from[i][j] == std::numeric_limits<int>::max() ?  maze.from[i][j] :  maze.from[i][j]+1;
        }
    }

    std::cout<<(maze_bb(maze)==std::numeric_limits<int>::max()?0:mejorCamino)<<std::endl;

    double end = clock();


    std::cout<<visit<<" "<<explored<<" "<<leaf<<" "<<unfeasible<<" "<<notPromising<<" "<<promisingButDiscarded<<" "<<updatedFromLeafs<<" "<<updatedFromPesimisticBounds<<std::endl;

    std::cout<<1000.0*(end-start)/CLOCKS_PER_SEC<<std::endl;

    if (maze.path2D) std::cout<< (mejorCamino != std::numeric_limits<int>::max() ? maze_parser(maze) :"NO EXIT\n");

    if(maze.path) std::cout<< (mejorCamino != std::numeric_limits<int>::max() ? maze_parser1D(maze) :  "<NO EXIT>\n");
    


    return 0;
}
