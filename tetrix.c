#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <ncursesw/ncurses.h>

#define WIDTH 10
#define HEIGHT 20

//structs

typedef struct coord
{
    int h;
    int w;
}
coord;

typedef struct piece
{
    coord center;
    coord x;
    coord y;
    coord z;

    char name_piece;
} 
piece;

typedef struct range
{
    bool matrix[4][4];
}
range;


// protótipos
void load_grid_to_buffer();
void load_grid();
void *update_screen();
void spawn_piece_screen(piece p, int height, int width);
void init_game();
void complete_fall(int velocity);
void solidify_block();
void update_frame_piece_falldown();
void *input_control();
piece create_piece(char name);
void move_pieces(char direction);
bool is_touching_edge(char edge);
bool is_touching_solid(char side);
void spawn_random_piece();
void end_game();
void eliminate_line();
bool is_complete_line(int i);
void start_gameplay(int level);
void gravity_solid(int line);
void rotate_piece();
piece get_piece_from_range(range r);
void clear_piece_from_grid();
void drop_piece_instantly();
void respawn_piece_screen(piece t);
bool is_respawnenable(piece p);

//Threads
pthread_t pth_update_scr; //thread update_screen
pthread_t pth_inp_ctrl; //thread input_control


// global
// 1 para bloco no ar
// 2 para bloco solidificados
int grid[HEIGHT][WIDTH];
const int EMPTY = 0;
const int BLOCK = 1;
const int SOLID = 2;

piece current_piece;

coord local_center_piece;
coord local_center_piece_aux;
int current_velocity = 0;
bool drop_piece = false;


int main(void)
{
    //inicializar o jogo com grid e atualização de tela
    init_game();
    start_gameplay(0);

    pthread_join(pth_update_scr, NULL);
    pthread_join(pth_inp_ctrl, NULL);

    end_game();
    return 0;
}


/**
 * Insere os valores do vetor bidimensional na telabuffer do Ncurses.
 */
void load_grid_to_buffer() 
{
    // código que usa a matriz grid
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (grid[i][j] == BLOCK)
            {

                addch(ACS_BLOCK);
                addch(' ');
            }
            else if (grid[i][j] == SOLID)
            {
                addch('#');
                addch(' ');
            }
            else
            {
                addch(' ');
                addch(' ');
            }
        }
        addch('\n');
    }
}


//colocando valores 0 em toda grid 10 x 20 
void load_grid()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            grid[i][j] = EMPTY;
        }
    }
}

/**
 * Função executada pela thread de atualização de tela.
 * Atualiza a tela a cada 60ms enquanto a variável global
 */

void *update_screen()
{
    while(true)
    {
        clear();
        move(0, 0);
        load_grid_to_buffer();
        refresh();
        napms(60);
    }
    return NULL;
}

// Drops a piece on the screen
void spawn_piece_screen(piece p, int height, int width)
{
    //CENTER
    int ch = p.center.h;
    int cw = p.center.w;

    // x
    int xh = p.x.h;
    int xw = p.x.w;

    // y  
    int yh = p.y.h;
    int yw = p.y.w;

    // z 

    int zh = p.z.h;
    int zw = p.z.w;

    //CENTER
    grid[height + ch][width + cw] = BLOCK;

    //get coord from center
    local_center_piece.h = height + ch;
    local_center_piece.w = width + cw;

    // x
    grid[height + xh][width + xw] = BLOCK;

    //get local from center_aux
    local_center_piece_aux.h = height + xh;
    local_center_piece_aux.w = width + xw;

    // y 
    grid[height + yh][width + yw] = BLOCK;

    // z
    grid[height + zh][width + zw] = BLOCK;
    return;
}

void init_game()
{
    // definindo estrutura de dados bidimensional ("tabela")
    initscr(); //inicializa o terminal no modo curses
    //raw(); // trava teclas ctrl + C e ctrl + z
    cbreak();
    noecho(); // para não imprimir caracteres desnecessários
    keypad(stdscr, true); // ativa teclas especiais]
    //nodelay(stdscr, true);
    // colocar valores do grid como 0
    load_grid(); // preenche os valores vazio em grid


    //criando atualização de tela
    pthread_create(&pth_update_scr, NULL, update_screen, NULL);
    pthread_create(&pth_inp_ctrl, NULL, input_control, NULL);
}

//aplica gravidade aos blocos
void complete_fall(int velocity)
{

    while(!drop_piece)
    {
        update_frame_piece_falldown(); // cria frame de atualização de queda
        //se após o frame o bloco estiver tocando  em algo
        if (is_touching_edge('b') || is_touching_solid('b'))
        {
            napms(1000);
            solidify_block();
            return;
        }
        //atrazo após a iteração
        napms(1000 - velocity * 10);
    }

    while (drop_piece)
    {
        update_frame_piece_falldown();
        if(is_touching_edge('b') || is_touching_solid('b'))
        {
            solidify_block();
            drop_piece = false;
            return;
        }
    }
    return;
}

//generate a piece: L, J, I, T, O, S, Z
piece create_piece(char name)
{
    piece p;

    switch (name)
    {
    case 'L':
        
        p.name_piece = 'L';

        p.center.h = 1;
        p.center.w = 1;
        
        // x
        p.x.h = 0;
        p.x.w = 1;

        // y
        p.y.h = 2;
        p.y.w = 1;

        //z
        p.z.h = 2;
        p.z.w = 2;

        break;
    
    case 'J':
        //name
        p.name_piece = 'J';

        // center
        p.center.h = 1;
        p.center.w = 1;

        // x 
        p.x.h = 0;
        p.x.w = 1;

        // y 
        p.y.h = 2;
        p.y.w = 1;

        // z
        p.z.h = 2;
        p.z.w = 0;

        break;

    case 'I':
        //name
        p.name_piece = 'I';

        //center 1
        p.center.h = 1;
        p.center.w = 1;

        //center 2
        p.x.h = 1;
        p.x.w = 2;

        // x
        p.y.h = 1;
        p.y.w = 0;

        // y
        p.z.h = 1;
        p.z.w = 3;
        break;

    case 'T':

        //name
        p.name_piece = 'T';

        //center
        p.center.h = 1;
        p.center.w = 1;

        // x
        p.x.h = 0;
        p.x.w = 1;

        // y
        p.y.h = 1;
        p.y.w = 0;

        // z
        p.z.h = 1;
        p.z.w = 2;

        break;

    case 'O':
        //name
        p.name_piece = 'O';

        //center
        p.center.h = 1;
        p.center.w = 1;

        // center 2
        p.x.h = 2;
        p.x.w = 1;

        // y
        p.y.h = 1;
        p.y.w = 2;

        // z
        p.z.h = 2;
        p.z.w = 2;
   
        break;

    case 'S':
        // name
        p.name_piece = 'S';

        // center
        p.center.h = 1;
        p.center.w = 1;

        // x 
        p.x.h = 1;
        p.x.w = 2;

        // y 
        p.y.h = 2;
        p.y.w = 0;

        // z
        p.z.h = 2;
        p.z.w = 1;
        
        break;

    case 'Z':
        //name
        p.name_piece = 'Z';

        // center
        p.center.h = 1;
        p.center.w = 1;

        // x 
        p.x.h = 1;
        p.x.w = 0;

        // y 
        p.y.h = 2;
        p.y.w = 1;

        // z
        p.z.h = 2;
        p.z.w = 2;
    
    default:
        break;
    }

    return p;
}

//Solidifica todos os blocos da tela
void solidify_block()
{
    for (int i = HEIGHT - 1; i >= 0; i--)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (grid[i][j] == BLOCK)
            {   
                grid[i][j] = SOLID;
            }
        }
    }
    napms(6);
}

// Cria frame de atualização da queda de uma peça
// a cada chamada a peça cai um bloco de altura
//return true se algum bloco se moveu, false caso nenhum bloco se moveu de lugar
void update_frame_piece_falldown()
{
    for (int i = HEIGHT - 2; i >= 0; i--)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (grid[i][j] == BLOCK)
            {
                if (grid[i + 1][j] == EMPTY)
                {
                    grid[i + 1][j] = BLOCK; // joga para a linha de baixo
                    grid[i][j] = EMPTY; // apaga o valor da linha atual
                }
            } 
        }
    }

    // update local_center_from_piece

    local_center_piece.h ++;
    local_center_piece_aux.h ++;

    return;
}

// Habilita teclas para controle
void *input_control()
{
    int key;

    while (true)
    {
        key = getch();

        if (key != ERR)
        {            
            switch (key)
            {
            case KEY_RIGHT:
                move_pieces('r');
                break;

            case KEY_LEFT:
                move_pieces('l');
                break;

            case KEY_DOWN:
                break;
            
            case KEY_UP:
                rotate_piece();
                break;

            case ' ':
                drop_piece_instantly();
                break;

            default:
                break;
            }

            halfdelay(20000);
        } 
    }   
    return NULL;
}

//move blocks to left or right use "l" or "r"
void move_pieces(char direction)
{
    //caso mova para a esquerda
    if (direction == 'l' && !is_touching_edge('l') && !is_touching_solid('l'))
    {
        for (int i = HEIGHT - 1; i >= 0; i--)
        {
            for (int j = 0; j < WIDTH ; j++)
            {
                if (grid[i][j] == BLOCK)
                {   
                    grid[i][j - 1] = BLOCK;
                    grid[i][j] = EMPTY;
                }
            }
            
        }

        //update local_center_piece
        local_center_piece.w --;
        local_center_piece_aux.w --;
    }
    //caso mova para direita
    else if (direction == 'r' && !is_touching_edge('r') && !is_touching_solid('r'))
    {
        for (int i = HEIGHT - 1; i >= 0; i--)
        {
            for (int j = WIDTH - 1; j >= 0 ; j--)
            {
                if (grid[i][j] == BLOCK)
                {
                    //pthread_mutex_lock(&mutex);
                    grid[i][j + 1] = BLOCK;
                    grid[i][j] = EMPTY;
                    //pthread_mutex_unlock(&mutex);
                }
            }
            
        }
        local_center_piece.w ++;
        local_center_piece_aux.w ++;
    }

    if (is_touching_edge('b') || is_touching_solid('b'))
    {
        napms(100);
        solidify_block();
    }
}
//edge left = 'l'
//edge right = 'r'
//edge bottom = 'b'
bool is_touching_edge(char edge)
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            // batendo nas borda?
            if (grid[i][j] == BLOCK)
            {
                if (j == 0 && edge == 'l')
                {
                    return true;
                }

                if (j == WIDTH - 1 && edge == 'r')
                {
                    return true;
                }

                if (i == HEIGHT - 1 && edge == 'b')
                {
                    return true;
                }
                
            }
        }
        
    }
    return false;
}

//left 'l'
//right 'r'
//bottom 'b'
// @param side 'l', 'r', 'b'
bool is_touching_solid(char side)
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (grid[i][j] == BLOCK)
            {
                //bottom
                if (i != HEIGHT - 1 && grid[i + 1][j] == SOLID && side == 'b')
                {
                    return true;
                }
                //left
                if (grid[i][j - 1] == SOLID && side == 'l')
                {
                    return true;
                }
                //right
                if (grid[i][j + 1] == SOLID && side == 'r')
                {
                    return true;
                }

            }
        }
        
    }
    
    return false;
}

void spawn_random_piece()
{
    //definindo semente aleatória
    srand(time(NULL));

    //array de char
    char letter[7] = {'I', 'J', 'L', 'T', 'O', 'S', 'Z'};

    current_piece = create_piece(letter[rand() % 7]);

    

    if (current_piece.name_piece == 'I' || current_piece.name_piece == 'O' || current_piece.name_piece == 'Z' || current_piece.name_piece == 'S')
    {
        spawn_piece_screen(current_piece, -1, 3);
    }
    else
    {
        spawn_piece_screen(current_piece, 0, 3);
    }
}

void end_game()
{
    keypad(stdscr, false);
    echo();
    nodelay(stdscr, false);
    endwin(); //libera memória alocada por initscr
}

void eliminate_line()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        if (is_complete_line(i))
        {
            for (int j = 0; j < WIDTH; j++)
            {
                grid[i][j] = EMPTY;
                napms(60);
                refresh(); 
            }
            gravity_solid(i);
        }
    }

}

bool is_complete_line(int i)
{
    for (int j = 0; j < WIDTH; j++)
    {
        if (grid[i][j] != SOLID)
        {
            return false;
        }
    }
    return true;
}

//fazar gravidade na linha dos blocos
void gravity_solid(int line)
{
    for (int i = line; i >= 1; i--)
    {
        for (int j = 0; j < WIDTH; j++)
        {       
            if( grid[i - 1][j] == SOLID)
            {
                grid[i][j] = grid[i - 1][j]; 
                grid[i - 1][j] = EMPTY;
            }
        }
        
    }
    
}
// @param level game
void start_gameplay(int level)
{
    // set velocity default
    
    while (true)
    {
        current_velocity = 60 + level;    
        spawn_random_piece();
        complete_fall(current_velocity);
        eliminate_line();
        napms(500);
    }
}

void rotate_piece()
{
    // 'O' isnt rotate
    if (current_piece.name_piece == 'O')
    {
        return;
    }

    //name
    char name_piece = current_piece.name_piece;

    //range to cut o piece
    range r;

    //fill r with empty spaces
    for (int i = 0; i < 4; i++)
    { 
        for (int j = 0; j < 4; j++)
        {
            r.matrix[i][j] = EMPTY;
        }
    }

    //t - temp - to copy a current piece falling in the screen
    piece t = current_piece;

    //put the piece into the matrix

    // center
    r.matrix[t.center.h][t.center.w] = BLOCK;

    // x
    r.matrix[t.x.h][t.x.w] = BLOCK;

    // y
    r.matrix[t.y.h][t.y.w] = BLOCK;

    // z
    r.matrix[t.z.h][t.z.w] = BLOCK;

    range r_temp = r;

    //rotate matrix
    if (name_piece == 'I' || name_piece == 'O')
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                r.matrix[i][j] = r_temp.matrix[3 - j][i];
            }   
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                r.matrix[i][j] = r_temp.matrix[2 - j][i];
            }
        }
    }

    //put matrix into them piece
    t = get_piece_from_range(r);

    //if this piece is spawnenable
    if (!is_respawnenable(t))
    {
        return;
    }
    
    //update current piece in the screen
    current_piece = t;

    //respawn piece in the screen
    respawn_piece_screen(t);

    
    return;
}
             
piece get_piece_from_range(range r)
{
    coord coords[4];

    int count = 0;

    piece temp_piece;

    if (current_piece.name_piece == 'I' || current_piece.name_piece == 'O')
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (r.matrix[i][j] == BLOCK && count < 4)
                {
                    coords[count].h = i;
                    coords[count].w = j;

                    count ++;
                }
            }        
        }

        temp_piece.center.h = coords[1].h;
        temp_piece.center.w = coords[1].w;

        temp_piece.x.h = coords[2].h;
        temp_piece.x.w = coords[2].w;

        temp_piece.y.h = coords[0].h;
        temp_piece.y.w = coords[0].w;

        temp_piece.z.h = coords[3].h;
        temp_piece.z.w = coords[3].w;

    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (r.matrix[i][j] == BLOCK && count < 3 && !(i == 1 && j == 1))
                {
                    coords[count].h = i;
                    coords[count].w = j;
                    count ++;
                }  
            }
        }

        temp_piece.center.h  = 1;
        temp_piece.center.w = 1;

        temp_piece.x.h = coords[0].h;
        temp_piece.x.w = coords[0].w;

        temp_piece.y.h = coords[1].h;
        temp_piece.y.w = coords[1].w;

        temp_piece.z.h = coords[2].h;
        temp_piece.z.w = coords[2].w;
    }

    //get name piece
    temp_piece.name_piece = current_piece.name_piece;

    return temp_piece;
}

//limpa a tela de qualquer peça
void clear_piece_from_grid()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (grid[i][j] == BLOCK)
            {
                grid[i][j] = EMPTY;
            }
        }
    }
}

//drop piece instantly
void drop_piece_instantly()
{    
    drop_piece = true;
}

void respawn_piece_screen(piece t)
{
    //get current coords piece
    coord local = local_center_piece;

    clear_piece_from_grid();

    int max_range = local.w;
    
    if (t.name_piece == 'I')
    {
        //create a exception to 'I' piece
        if (t.center.h == 1 && t.x.w == 2)
        {
            local.h -= 1;
            local.w -= 1;
        }
        else if (t.center.h == 2 && t.x.w == 2)
        {
            local.h -= 1; 
            local.w -= 2;
        }
        else if (t.center.h == 1 && t.x.w == 1)
        {
            local.h -= 2;
            local.w -= 1;
        }
        max_range += 4;
    }
    else
    { 
        local.h -= 1; 
        local.w -= 1;
        max_range += 3;
    }

    
    if (max_range > WIDTH - 1)
    {
        local.w -= (max_range % WIDTH - 1);
    }
    else if (local.w < 0)
    {
        local.w -= local.w;
    }

    if (local.h < 0)
    {
        local.h -= local.h;
    }
    
    spawn_piece_screen(t, local.h, local.w);
    return;
}

bool is_respawnenable(piece p)
{
    coord local = local_center_piece;

    if (p.name_piece == 'I')
    {
        //create excetion to 'I' piece
        if (p.center.h == 1 && p.x.w == 2)
        {
            local.h -= 1;
            local.w -= 1;
        }
        else if (p.center.h == 2 && p.x.w == 2)
        {
            local.h -= 1;
            local.w -= 2;
        }
        else if (p.center.h == 1 && p.x.w == 1)
        {
            local.h -= 2;
            local.w -= 1;
        }
    }
    else
    {
        local.h -= 1;
        local.w -= 1;
    }

    //center
    if (grid[local.h + p.center.h][local.w + p.center.w] == SOLID)
    {
        return false;
    }

    // x
    if (grid[local.h + p.x.h][local.w + p.x.w] == SOLID)
    {
        return false;
    }
    // y
    if (grid[local.h + p.y.h][local.w + p.y.w] == SOLID)
    {
        return false;
    }
    // z
    if (grid[local.h + p.z.h][local.w + p.z.w] == SOLID)
    {
        return false;
    }

    return true;
}