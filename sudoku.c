#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define PY_SSIZE_T_CLEAN
#include <python3.10/Python.h>

typedef unsigned char byte;

// Essential tile struct
// id: a serial number for storing the tile's index in the 1-dimensional board array.
// 0 is the top-left tile, 80 is the bottom-right, numbered left-right up-down
typedef struct tile{
    byte id;
    byte num;
    byte* guesses;
}Tile;

Tile** 		board;
Tile**** 	tile_indices;
byte solved_tiles = 0;

// Do math on the tile's ID to find its neighbors from the indices
// 0: neighboring row
// 1: neighboring column
// 2: neighboring region
Tile**
get_neighbors(Tile t, byte mode){
    Tile*** index = tile_indices[mode];
    byte s = t.id;
    if(!mode){
        return index[s/9];
    }else if(mode == 1){
        return index[s%9];
    }else{
        return index[s/3%3+s/27*3];
    }
}

// Use bitwise ops to insert the corresponding bit to the guess bytes
void
add_guess(Tile* t, byte g){
	if(g < 9){
		t->guesses[1] |= (1 << (g - 1));
	}else{
		t->guesses[0] = 1;
	}
}

// Use the opposite ops to remove the corresponding bit
void 
remove_guess(Tile* t, byte g){
	if(g < 9){
		t->guesses[1] &= (~(1 << (g - 1)));
	}else{
		t->guesses[0] = 0;
	}
}

// Count all 1-bits in a 9-bit segment, where the first bit is 
// the rightmost of a byte and the next 8 constitute a byte
byte
ones(byte a, byte b){
	byte count = 0;
	if(a){
		count++;
	}
	while(b){
		count += b & 1;
		b >>= 1;
	}
	return count;
}

// Intended for use with single-candidate tiles
// Will return the corresponding number for the first bit it finds
// Scans from 9 down to 1, returns 0 if no 1's are found in the bytes
byte
scan(byte a, byte b){
	if(a){
		return 9;
	}else{
		byte mask = 0x80;
		byte r = 8;
		while(r){
			if(mask & b){
				return r;
			}else{
				mask >>= 1;
				r--;
			}
		}
	}
	return 0;
}

// Set a tile's number and remove guesses for that number from all 20 adjacent tiles
void
confirm(Tile* t, byte g){
    t->num = g;
    for(byte i = 0; i < 3; i++){
        Tile** neighbors = get_neighbors(*t, i);
        for(byte j = 0; j < 9; j++){
			Tile* n = neighbors[j];
            remove_guess(n, g);
        }
    }
    t->guesses[0] = 0;
    t->guesses[1] = 0;
	solved_tiles++;
}

// Verify continuity with Sudoku rules
// i.e. check all neighbors for redundant numbering
byte
validate(Tile t){
	for(byte i = 0; i < 3; i++){
		Tile** neighbors = get_neighbors(t, i);
		for(byte j = 0; j < 9; j++){
            Tile n = *(neighbors[j]);
            if(t.id != n.id && t.num == n.num){
                return 0;
            }
		}
	}
	return 1;
}

void
print_bits(byte byte0, byte byte1){
	if(byte0){
		printf("1 ");
	}else{
		printf("0 ");
	}
	byte mask = 0x80;
	while(mask){
		if(mask & byte1){
			printf("1");
		}else{
			printf("0");
		}
		mask >>= 1;
	}
	printf("  %X%X", byte0, byte1);
}

byte
get_hidden_single(Tile t){
	// Not eligible for hidden single if it's already solved!
	if(t.num){
		return 0;
	}

	// Go through row, column and region modes for the get_neighbors function
	for(byte i = 0; i < 3; i++){
		Tile** neighbors = get_neighbors(t, i);

		// Take the NOTs of the guess bytes. this is useful later.
		byte gbyte0 = ~t.guesses[0];
		byte gbyte1 = ~t.guesses[1];

		// OR together the guess bytes with the guess bytes of everything else
		for(byte j = 0; j < 9; j++){
			Tile n = *(neighbors[j]);
			if(t.id != n.id){
				gbyte0 |= n.guesses[0];
				gbyte1 |= n.guesses[1];
			}
		}

		// If there is a zero bit in the guess bytes at this point, we've found a hidden single!

		/*
		EXAMPLE:

		Consider this:

			 1	  | 1    |
			 4	  | 4 5  |  3
			   8  |      |
		    ------+------+------
				  |		 |
			   2  |   6  |  7
				  |		 |
			------+------+------
			 1	  | 1	 | 1
			 4	  | 4	 | 4
			   8 9|     9|   8

		...Denoting a region, where 3, 2, 6 and 6 are solved, and the top-left, top-middle, and all 3 bottom
		tile are unclear and each have guesses remaining.

		Notice that there is only a 5 in one tile.

		Examine the guess bits, where we've NOT-ed the top middle tile:

		TOP LEFT		0 1 0 0 0 1 0 0 1
		[NOT] TOP MID   1 1 1 1 0 0 1 1 0	
		TOP RIGHT		0 0 0 0 0 0 0 0 0
		MID LEFT		0 0 0 0 0 0 0 0 0
		MID				0 0 0 0 0 0 0 0 0
		MID RIGHT		0 0 0 0 0 0 0 0 0
		BOTTOM LEFT		1 1 0 0 0 1 0 0 1
		BOTTOM MID		1 0 0 0 0 1 0 0 1
		BOTTOM RIGHT	0 1 0 0 0 1 0 0 1

		Now OR all of these bits together. What do we get?

		COMPOSITE			1 1 1 1 0 1 1 1 1
		[NOT] COMPOSITE		0 0 0 0 1 0 0 0 0

		...And look! There's a 1-bit in the 5th slot! That means 5 is the hidden single!
		(Same conclusion you'd reach from looking at the table above)
		
		*/

		// Take the NOTs of the guess bytes again
		gbyte0 = ~gbyte0;
		gbyte1 = ~gbyte1;

		// Return the hidden single, if it exists
		if(ones(gbyte0, gbyte1) == 1){
			return scan(gbyte0, gbyte1);
		}
	}
	return 0;
}

// Print the board in traditional Sudoku format

void
print_board(){
	printf("\n");
	for(byte i = 0; i < 81; i++){
		byte tilenum = board[i]->num;
		if(!((i+1)%9)){
			printf("%d\n", tilenum);
		}else if((i+1)%3 == 0){
			printf("%d | ", tilenum);
		}else{
			printf("%d ", tilenum);
		}
		if(!((i+1)%27) && i < 80){
			printf("------+-------+------\n");
		}
	}
	printf("\n");
}

// Print in-depth info about the guesses of each tile.

void
print_guesses(){
	printf("GUESS LIST\n\n");
	for(byte i = 0; i < 81; i++){
		Tile t = *(board[i]);
		byte ga = t.guesses[0];
		byte gb = t.guesses[1];
		byte oc = ones(ga, gb);
		printf("\t%d\t%d\tHS [%d]\t", t.id, oc, get_hidden_single(t));
		print_bits(ga, gb);
		if(oc == 1){
			printf("\t%d\n", scan(ga, gb));
		}else if(!oc){
			printf("\tsolved\n");
		}else{
			printf("\tambiguous\n");
		}
	}
}

// Self-explanatory. Initialize all tiles as though the entire board's empty.

void
initialize_board(){
	for(byte i = 0; i < 81; i++){
		solved_tiles = 0;
		Tile* tp = board[i];
		tp->id = i;
		tp->num = 0;
		tp->guesses[0] = 1;
		tp->guesses[1] = 255;
	}
}

// Load a file (name given by fname) into the board structure.

void
load_board_from_file(char* fname){
	initialize_board();

	char* filebuf = calloc(91, 1);
	
	FILE* file = fopen(fname, "r");

    if(!file){
		printf("File not found!\n");
		exit(1);
	}

	fseek(file, 0L, SEEK_END);
	long size = ftell(file);
	rewind(file);

	if(size != 89){
		printf("Improper file! Not 89 bytes!\t%ld\n", size);
		exit(1);
	}

	fread(filebuf, 1, 90, file);
    fclose(file);

	char c = '0';

    byte i = 0, j = 0;

    while(c){
		c = filebuf[i];
		if(c == 48){
			j++;
		}else if(49 <= c && c <= 57){
			Tile* tp = board[j];
			confirm(tp, c-48);
			if(!validate(*tp)){
				printf("Invalid input file! Tile %d is redundant!\n", tp->id);
                exit(1);
			}
			j++;	
		}else if(!c){
			break;
		}else if(c != 10){
			printf("Invalid character in file!\n");
			exit(1);
		}

		i++;
	}

	free(filebuf);
}

/*

	1. Find minimum nonzero guess count
	2. Scan for tiles with that count. Append them to a linked list
	3. Select a random one from the linked list and make a random viable guess

*/

// Simple linked-list implementation for use with guessing

typedef struct tile_linked_list_node{
	Tile* tile_ptr;
	struct tile_linked_list_node* next;
}TLLNode;

typedef struct tile_linked_list{
	TLLNode* head;
	TLLNode* tail;
}TLList;

void TLL_append(TLList* tl, Tile* tp){
	TLLNode* new_node = malloc(sizeof(TLLNode));
	new_node->tile_ptr = tp;
	new_node->next = NULL;

	if(!tl->head){
		tl->head = tl->tail = new_node;
	}else{
		tl->tail->next = new_node;
		tl->tail = tl->tail->next;
	}
}

// Guess function
Tile*
guess(){
	byte guess_min = 9;

	for(byte i = 0; i < 81; i++){
		// Get a pointer to tile i
		Tile* ti = board[i];

		// Get ti's "ones"
		byte tio = ones(ti->guesses[0], ti->guesses[1]);

		// Loop to find minimum "ones" held by ANY tile.
		// Statistically, a correct guess is most likely made on a tile with the least amount of possibilities.
		if(tio && tio < guess_min){
			guess_min = tio;
		}
		//printf("%d\n", guess_min);
	}

	// Find how many tiles have that same min guess count
	byte min_tile_count = 0;

	TLList TLL;
	TLL.head = TLL.tail = NULL;

	// Since we're not entirely sure at this point how many tiles share that minimum, it's a little faster
	// if we append every eligible tile we find to a linked list without bounded length.
	for(byte i = 0; i < 81; i++){
		Tile* ti = board[i];
		byte tio = ones(ti->guesses[0], ti->guesses[1]);
		if(tio == guess_min){
			min_tile_count++;
			TLL_append(&TLL, board[i]);
		}
	}
	
	// Prepare for random number generation
	srand(time(NULL));

	// Get head of linked list
	TLLNode* current = TLL.head;
	// printf("MTC: %d\n", min_tile_count);

	// Jump a random number of times through the linked list,
	// effectively selecting a random tile with the min guess count
	byte jumps = 0;
	if(min_tile_count){
		jumps = rand()%min_tile_count;
	}else{
		return NULL;
	}
	for(byte i = 0; i < jumps; i++){
		current = current->next;
	}

	Tile* ctile = current->tile_ptr;
	// printf("%d\n", ctile->id);
	
	byte guesses_arr[guess_min];
	byte idx = 0, mask = 1;

	for(byte i = 1; i < 9; i++){
		//printf("%d\t%d\n", i, !!(mask & ctile->guesses[1]));

		if(mask & ctile->guesses[1]){
			guesses_arr[idx] = i;
			idx++;
		}

		mask <<= 1;
	}

	if(ctile->guesses[0]){
		guesses_arr[idx] = 9;
	}

	// printf("9\t%d\n", !!ctile->guesses[0]);

	/*
	printf("[%d]\n\n> ", ctile->id);
	for(byte i = 0; i < guess_min; i++){
		printf("%d\t", guesses_arr[i]);
	}
	printf("\n");
	*/

	// printf("%d\n", ones(ctile->guesses[0], ctile->guesses[1]));

	// printf("%d\n", guess_min);
	confirm(ctile, guesses_arr[rand()%guess_min]);

	TLLNode* curr = TLL.head;
	TLLNode* next = TLL.head->next;

	while(next){
		free(curr);
		curr = next;
		next = next->next;
	}

	free(curr);

	return ctile;
}

byte
main(int argc, char** argv){
    if(argc != 2){
		printf("Need exactly one path to a .sudoku file or similar!\n");
		return 1;
	}

    // HEAP ALLOCATION & SETUP

    board = malloc(sizeof(Tile*)*81);
    for(byte i = 0; i < 81; i++){
        board[i] = malloc(sizeof(Tile));
        board[i]->guesses = malloc(2);
    }

	initialize_board();

    // Plot out the board indices (a sort of cache for quickly obtaining a row/region/column)
    tile_indices = malloc(sizeof(Tile***)*3);
    for(byte i = 0; i < 3; i++){
        tile_indices[i] = calloc(sizeof(Tile**), 9);
        for(byte j = 0; j < 9; j++){
            tile_indices[i][j] = malloc(sizeof(Tile*)*9);
            Tile** nine = tile_indices[i][j];
            
            if(!i){
                for(byte k = 0; k < 9; k++){
                    nine[k] = board[j*9+k];
                    //printf("%d\n", nine[k]->id);
                }
            }else if(i == 1){
                for(byte k = 0; k < 9; k++){
                    nine[k] = board[j+9*k];
                    //printf("%d\n", nine[k]->id);
                }
            }else{
                for(byte k = 0; k < 9; k++){
                    byte x = j*9+k;
                    nine[k] = board[x%3+3*(x/9%9)+9*(x/3%3)+18*(x/27%9)];
                    //printf("%d\n", nine[k]->id);
                }
            }
        }
    }

	load_board_from_file(argv[1]);

    print_board();
	//printf("%d\n", solved_tiles);
	//print_guesses();

	byte 	guessing = 0, 
			single_candidate_count = 0, 
			hidden_single_count = 0,
			guess_count = 0,
			progress_this_iteration = 0,	// Treat this like a bool
			solved_pre = solved_tiles;

	while(solved_tiles < 81){
		if(!guessing){
			progress_this_iteration = 0;
			for(byte i = 0; i < 81; i++){
				Tile* tp = board[i];
				Tile t = *tp;
				byte a = t.guesses[0];
				byte b = t.guesses[1];

				// Single candidate tactic
				if(ones(a, b) == 1){
					confirm(tp, scan(a, b));
					single_candidate_count++;
					progress_this_iteration = 1;

				// Hidden single tactic
				}else{
					byte hs = get_hidden_single(t);
					if(hs){
						confirm(tp, hs);
						hidden_single_count++;
						progress_this_iteration = 1;
					}
				}
			}
			if(!progress_this_iteration){
				guessing = 1;
			}
		}else{
			if(guess()){
				guess_count++;
			}else{
				// printf("dammit...\n");
				load_board_from_file(argv[1]);
				single_candidate_count = hidden_single_count = guess_count = progress_this_iteration = 0;
			}
			guessing = 0;
		}
	}

	//printf("%d\n", solved_tiles);

	printf("\n=== SOLVED ===\n\nTiles solved on load\t\t\t%d\nTiles solved w/ Single Candidate:\t%d\nTiles Solved w/ Hidden Single:\t\t%d\nTiles solved w/ Guessing:\t\t%d\n", solved_pre, single_candidate_count, hidden_single_count, guess_count);
	print_board();
	//print_guesses();

	// guess();

    // HEAP CLEARING

    // free(filebuf);

    for(byte i = 0; i < 3; i++){
        for(byte j = 0; j < 9; j++){
            free(tile_indices[i][j]);
        }
        free(tile_indices[i]);
    }
    free(tile_indices);

    for(byte i = 0; i < 81; i++){
        Tile* tp = board[i];
        free(tp->guesses);
        free(tp);
    }

    free(board);
}