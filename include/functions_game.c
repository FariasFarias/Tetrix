#include "functions_game.h"

const int EMPTY = 0;
const int BLOCK = 1;
const int SOLID = 2;

//Thread ini to update screen
pthread_t pth_update_scr; 

//Thread to recive inputs control (UP, DOWN, RIGHT, LEFT, ESPACE)
pthread_t pth_inp_ctrl; //thread input_control

//ptheard mutex
//update_screen
pthread_mutex_t mutex_screen_update = PTHREAD_MUTEX_INITIALIZER;

//update_grid
pthread_mutex_t mutex_grid_update = PTHREAD_MUTEX_INITIALIZER;

//update_global_var
pthread_mutex_t mutex_globalvar_update = PTHREAD_MUTEX_INITIALIZER;

// const to specify type block

// grid to save struct game
int grid[HEIGHT][WIDTH];

// global Variables
piece current_piece; //piece atual 
piece hold_piece; //
piece row_piece[3];//
coord local_center_piece; //
coord local_center_piece_aux;
int current_velocity = 0;
bool drop_piece = false; //drop piece instantly in game
bool is_gameplay_run = true; 
bool is_piece_falling = false;
bool is_hold_enable = false;

//points
int line_points = 0;
int level_points = 1;
int score_points = 0;
int next_level_lines = 10;
int next_level_score = 100;

/*
 * Insere os valores do vetor bidimensional na telabuffer do Ncurses.
 */
//move(linha, coluna)
void load_grid_to_buffer() 
{
    // código que usa a matriz grid
    for (int i = 0; i < HEIGHT; i++)
    {
        //line, column
        int curser[2] = {3 + i, 15 + (COLS / 3) - 4};

        for (int j = 0; j < WIDTH; j++)
        {   
            move(curser[0], curser[1] + j);
            if (grid[i][j] == BLOCK)
            {
                addch(ACS_BLOCK);
            }
            else if (grid[i][j] == SOLID)
            {
                addch('#');
            }
            else
            {
                addch('.');
            }
            curser[1] ++;
        }
        
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
    while(is_gameplay_run)
    {
        load_grid_to_buffer();
        update_interface_game_points(score_points, line_points, level_points);
        update_interface_pieces();
        print_hold_piece();
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


int init_game()
{
    // definindo estrutura de dados bidimensional ("tabela")
    initscr(); //inicializa o terminal no modo curses
    raw(); // trava teclas ctrl + C e ctrl + z
    noecho(); // para não imprimir caracteres desnecessários
    keypad(stdscr, true); // ativa teclas especiais]
    //nodelay(stdscr, true);
    // colocar valores do grid como 0
    load_grid(); // preenche os valores vazio em grid
    curs_set(false);
    clear();

    //init interface game
    int rtn_init_interface = init_interface_game();
    if ( rtn_init_interface != 0)
    {
        return -1;
    }
    
    //criando atualização de tela
    int rtn_update_scr = pthread_create(&pth_update_scr, NULL, update_screen, NULL);

    //criar espera de inputs
    int rtn_in_ctrl = pthread_create(&pth_inp_ctrl, NULL, input_control, NULL);

    //caso ocorra algum erro ao inicializa a threads
    if (rtn_update_scr != 0 || rtn_in_ctrl != 0)
    {
        return -1;
    }
    
    start_gameplay();
    return exit_game();

}


//aplica gravidade aos blocos
int complete_fall(int velocity)
{
    is_piece_falling = true;
    is_hold_enable = true;
    while(!drop_piece && is_gameplay_run)
    {
        update_frame_piece_falldown(); // cria frame de atualização de queda
        //se após o frame o bloco estiver tocando  em algo
        //atrazo após a iteração
        napms(1000 - velocity * 10);
    
        
        if (solidify_if_touch_bottom() || !is_piece_falling)
        {
            is_piece_falling = false;
            return 0;
        }
    }

    while (drop_piece && is_gameplay_run)
    {
        update_frame_piece_falldown();
        if(solidify_if_touch_bottom() || !is_piece_falling)
        {
            drop_piece = false;
            is_piece_falling = false;
            return 0;
        }
    }

    return 0;
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

    while (is_gameplay_run)
    {
        //lock
        pthread_mutex_lock(&mutex_screen_update);
        key = getch();
        pthread_mutex_unlock(&mutex_screen_update);
        //unlock
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
                precipitate_piece();
                break;
            
            case KEY_UP:
                rotate_piece();
                break;

            case ' ':
                drop_piece_instantly();
                break;

            case 27:
            case 26:
            case 3:
                interrupt_game();
                break;

            case 'q':
            case 'Q':
                use_hold_piece();
                break;

            default:
                break;
            }

            halfdelay(30000);
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
        pthread_mutex_lock(&mutex_grid_update);
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
        pthread_mutex_unlock(&mutex_grid_update);

        //update local_center_piece
        local_center_piece.w --;
        local_center_piece_aux.w --;
    }
    //caso mova para direita
    else if (direction == 'r' && !is_touching_edge('r') && !is_touching_solid('r'))
    {
        pthread_mutex_lock(&mutex_grid_update);
        for (int i = HEIGHT - 1; i >= 0; i--)
        {
            for (int j = WIDTH - 1; j >= 0 ; j--)
            {
                if (grid[i][j] == BLOCK)
                {
                    grid[i][j + 1] = BLOCK;
                    grid[i][j] = EMPTY;
                }
            }
            
        }
        local_center_piece.w ++;
        local_center_piece_aux.w ++;
        pthread_mutex_unlock(&mutex_grid_update);
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

    // empty piece
    piece empty_piece;
    empty_piece.name_piece = ' ';

    while (current_piece.name_piece == ' ' || row_piece[0].name_piece  == ' ' || row_piece[1].name_piece  == ' ' || row_piece[1].name_piece  == ' ')
    {
        piece p = create_piece(letter[rand() % 7]);


        if (row_piece[1].name_piece == ' ')
        {
            row_piece[1] = row_piece[2];
            row_piece[2] = empty_piece;
        }

        if (row_piece[0].name_piece == ' ')
        {
            row_piece[0] = row_piece[1];
            row_piece[1] = empty_piece;
        }

        if (current_piece.name_piece == ' ')
        {
            current_piece = row_piece[0];
            row_piece[0] = empty_piece;
        }

        if (row_piece[2].name_piece == ' ')
        {
            row_piece[2] = p;
        }
    }
    


    if (current_piece.name_piece == 'I' || current_piece.name_piece == 'O' || current_piece.name_piece == 'Z' || current_piece.name_piece == 'S')
    {
        spawn_piece_screen(current_piece, -1, 3);
    }
    else
    {
        spawn_piece_screen(current_piece, 0, 3);
    }
}


int exit_game()
{

    pthread_join(pth_update_scr, NULL);
    pthread_join(pth_inp_ctrl, NULL);
    
    keypad(stdscr, false);
    echo();
    nodelay(stdscr, false);
    noraw();
    
    return endwin();
}


void eliminate_line()
{
    int count_of_total_lines = 0;
    for (int i = 0; i < HEIGHT; i++)
    {
        if (is_complete_line(i))
        {
            count_of_total_lines ++;

            for (int j = 0; j < WIDTH; j++)
            {
                grid[i][j] = EMPTY;
                napms(60);
                refresh(); 
            }
            gravity_solid(i);
            line_points ++;
            next_level_lines --;
        }
    }
    

    if(count_of_total_lines >= 2)
    {
        next_level_score -= 10 * (count_of_total_lines + 1) * level_points;
        score_points += 10 * (count_of_total_lines + 1) * level_points;
    }
    else
    {
        next_level_score -= 10 * count_of_total_lines * level_points;
        score_points += 10 * count_of_total_lines * level_points;
    }

    if(next_level_lines <= 0)
    {
        next_level_lines = 10;
        level_points ++;
    }

    if(next_level_score <= 0)
    {
        next_level_score = 100;
        level_points ++;
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
void start_gameplay()
{
    //put empty piece in hold
    piece empty_piece;
    empty_piece.name_piece = ' ';
    
    hold_piece = empty_piece;
    row_piece[0] = empty_piece;
    row_piece[1] = empty_piece;
    row_piece[2] = empty_piece;
    current_piece = empty_piece;

    // set velocity default
    while (is_gameplay_run)
    {
        current_velocity = 40;
        if (!is_piece_falling)
        {
            spawn_random_piece();
            complete_fall(current_velocity + level_points * 2);
            current_piece = empty_piece;
        }
        eliminate_line();
        napms(600);
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
    napms(900);
    drop_piece = false;
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


bool solidify_if_touch_bottom()
{
    if (is_touching_edge('b') || is_touching_solid('b'))
    {
        solidify_block();
        return true;
    }
    return false;
}


//init interface from game
int init_interface_game()
{
    //open file interface "gamescreen.txt"
    FILE *file;
	char line[49];
	file = fopen("src/gamescreen.txt", "r");

	// if erro in file
	if(file == NULL)
	{
		printw("Erro ao carregar o arquivo.");
		return -1 ;
	}

    //print file "gamescreen.txt"
    int r = 0;
    int c = (COLS / 3) - 4;
    
	while (fgets(line, sizeof(line), file) != NULL)
	{
        move(r, c);
        pthread_mutex_lock(&mutex_screen_update);
		for (int j = 0, n = strlen(line); j < n; j++)
		{   
			switch(line[j])
			{
				case 'p':
					addch(ACS_ULCORNER);
					break;
				case '-':
					addch(ACS_HLINE);
					break;
				case 'q':
					addch(ACS_URCORNER);
					break;
				case 'b':
					addch(ACS_LLCORNER);
					break;
				case 'd':
					addch(ACS_LRCORNER);
					break;
				case 'i':
					addch(ACS_VLINE);
					break;
				case 'e':
					addch(ACS_LTEE);
					break;
				case '3':
					addch(ACS_RTEE);
					break;
				default:
					addch(line[j]);
					break;
			}
            pthread_mutex_unlock(&mutex_screen_update);
		}
        r++;
    }
	//close file
	fclose(file);

	return 0;
}


void update_interface_game_points(int score, int lines, int level)
{
    char str_line[8];
    sprintf(str_line, "%7d", lines);

    char str_score[8];
    sprintf(str_score, "%7d", score);

    char str_level[8];
    sprintf(str_level, "%7d", level);

    
    mvprintw(17, 5 + (COLS / 3) - 4,"%s", str_line);
    mvprintw(21 , 5 + (COLS / 3) - 4,"%s", str_level);
    mvprintw(13, 5 + (COLS / 3) - 4 ,"%s",str_score);
    return;
}


void precipitate_piece()
{
    update_frame_piece_falldown();
    if (solidify_if_touch_bottom())
    {
        is_piece_falling = false;
        return;
    }
}

bool lose_game()
{   
    bool rtn = false;

    for (int i = 0; i < 1; i++)
    {
        for (int j = 3; j < 7; j++)
        {
            if (grid[i][j] == SOLID)
            {
                rtn = true;
            }
        }
    }
    
    return rtn;
}

void gameover()
{
    //todo
    mvprintw(10, 10,"Game over");
}


void interrupt_game()
{
    pthread_mutex_lock(&mutex_globalvar_update);
    is_gameplay_run = false;
    pthread_mutex_unlock(&mutex_globalvar_update);
}


void update_interface_pieces()
{
    // 0
    print_piece_screen(row_piece[0], 4, 38);
    // 1
    print_piece_screen(row_piece[1], 8, 38);

    // 2
    print_piece_screen(row_piece[2], 12, 38);
}


void print_piece_screen(piece p, int height, int width)
{
    ///clear local
    for (int i = height; i < 3 + height; i++)
    {
        for (int j = width + (COLS /3) - 5; j < width + 7 + (COLS /3) - 5; j++)
        {
            mvprintw(i, j, " ");
        }
    }
    
    mvprintw(height + p.center.h, width + p.center.w + (COLS / 3) - 4, "#");
    mvprintw(height + p.x.h, width + p.x.w + (COLS / 3) - 4, "#");
    mvprintw(height + p.y.h, width + p.y.w +  (COLS / 3) - 4, "#");
    mvprintw(height + p.z.h, width + p.z.w + (COLS / 3) - 4, "#");
}


void use_hold_piece()
{
    piece empty_piece;
    empty_piece.name_piece = ' ';

    if (hold_piece.name_piece == ' ' && is_piece_falling)
    {
        hold_piece = create_piece(current_piece.name_piece);
        clear_piece_from_grid();
        current_piece = row_piece[0];
        is_piece_falling = false;
    }
    else if (is_hold_enable && is_piece_falling)
    {
        piece temp = create_piece(current_piece.name_piece);
        clear_piece_from_grid();
        current_piece = hold_piece;
        hold_piece = temp;

        if (current_piece.name_piece == 'I' || current_piece.name_piece == 'O' || current_piece.name_piece == 'Z' || current_piece.name_piece == 'S')
        {
            spawn_piece_screen(current_piece, -1, 3);
        }
        else
        {
            spawn_piece_screen(current_piece, 0, 3);
        }

        is_hold_enable = false;
        is_piece_falling = true;
    }
}


void print_hold_piece()
{
    piece p = hold_piece;
    int height = 4;
    int width = 7+ (COLS / 3) - 5;
    
    for (int i = height; i < height + 3; i++)
    {
        for (int j = width; j < 5 + width; j++)
        {
            mvprintw( i, j, " ");
        }
        
    }
    


    mvprintw(height + p.center.h, width + p.center.w , "#");
    mvprintw(height + p.x.h, width + p.x.w, "#");
    mvprintw(height + p.y.h, width + p.y.w, "#");
    mvprintw(height + p.z.h, width + p.z.w, "#");
}

int menu()
{
    // Inicia o modo ncurses
    initscr();

    // Desabilita a exibição de caracteres digitados
    noecho();

    // Habilita a leitura de teclas de função (F1, F2, etc.) e de setas
    keypad(stdscr, TRUE);

    // Oculta o cursor
    curs_set(false);

    // Define as opções do menu
    char *menu_choices[] = {"Start Game", "Instructions", "About", "Exit"};
    int n_choices = sizeof(menu_choices) / sizeof(char *);

    // Define a posição inicial do cursor
    int highlight = 1;

    // Loop principal do menu
    while (1) {
        // Limpa a tela
        clear();

        // Imprime o título do menu
        mvprintw(2, (COLS / 3),"TETRIX\n\n");

        // Imprime as opções do menu, destacando a opção selecionada
        for (int i = 1; i <= n_choices; i++) {
            if (i == highlight) {
                attron(A_REVERSE);
            }
            mvprintw(i + 5, (COLS / 3), "%d. %s\n", i, menu_choices[i-1]);
            attroff(A_REVERSE);
        }

        // Atualiza a tela
        refresh();

        // Aguarda o usuário selecionar uma opção
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                // Move o cursor para cima
                if (highlight == 1) {
                    highlight = n_choices;
                } else {
                    highlight--;
                }
                break;
            case KEY_DOWN:
                // Move o cursor para baixo
                if (highlight == n_choices) {
                    highlight = 1;
                } else {
                    highlight++;
                }
                break;
            case 10:  // 10 é o valor ASCII da tecla "Enter"
                // Processa a opção selecionada
                switch (highlight) {
                    case 1:
                        // TODO: implementar a seção "Start Game"
                        endwin();
                        return init_game();
                        break;
                    case 2:
                        // TODO: implementar a seção "Instructions"
                        show_instructions();
                        break;
                    case 3:
                        // TODO: implementar a seção "About"
                        show_about();
                        break;
                    case 4:
                        // Sai do programa
                        endwin();
                        return 0;
                }
                break;
        }
    }
}

void show_instructions()
{
    clear();
    mvprintw(0 + 4, COLS/ 3, "======================Intructions======================");
    mvprintw(2 + 4, COLS/ 3, " - Use left and right arrows to move");
    mvprintw(3 + 4, COLS/ 3, " - Use up arrow to rotate piece");
    mvprintw(4 + 4, COLS/ 3, " - Use down arrow to accelerate");
    mvprintw(5 + 4, COLS/ 3, " - Hit space to drop piece");
    mvprintw(6 + 4, COLS/ 3, " - Use <Q> to hold a piece");
    getch();
}
void show_about()
{
    clear();
    mvprintw(0 + 4, COLS/ 3, "=========================About=========================");
    mvprintw(2 + 4, COLS/ 3, "Create by @FariasFarias");
    mvprintw(3 + 4, COLS/ 3, "GitHub > https://github.com/FariasFarias");
    mvprintw(4 + 4, COLS/ 3, "Source > https://github.com/FariasFarias/Tetrix");
    mvprintw(4 + 4, COLS/ 3, "");
    getch();
}