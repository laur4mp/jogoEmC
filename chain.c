//gcc -m32 jogo.c -o jogo.exe -I"C:\allegro-5.0.10-mingw-4.7.0\include" -L"C:\allegro-5.0.10-mingw-4.7.0\lib" -lallegro-5.0.10-monolith-mt

//mingw32-make

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#define NUM_ENEMIES 15
#define TIRO_INATIVO 0
#define TIRO_ATIVO 1
#define RAIO_TIRO 100
#define TEMPO_TIRO 2.0
#define BORDA_TIRO 4
#define SCORE_PENALTY 10

const float FPS = 100;  

const int SCREEN_W = 960;
const int SCREEN_H = 540;
const int HERO_W = 30; //largura do heroi
const int HERO_H = 40; //altura do heroi

float tempoJogo = 0; 
float dificuldade = 1.0; 



ALLEGRO_COLOR BKG_COLOR;
ALLEGRO_FONT *FONT_32;
ALLEGRO_SAMPLE *pontos = NULL;
ALLEGRO_SAMPLE *gameover = NULL;
ALLEGRO_AUDIO_STREAM *fundo = NULL;



typedef struct Tiro {
	float x, y;
	float raio;
	int modo;
	float timer;
	ALLEGRO_COLOR cor;
} Tiro;

typedef struct Ship {
	float x, y;
	int vel;	
	ALLEGRO_COLOR cor;
	Tiro tiro;
} Ship;

typedef struct Hero {
	Ship ship;
	int dir_x;
	int dir_y;
	float score;
} Hero;


typedef struct Enemy {
	Ship ship;
	float raio;
	int active;
	Tiro explosao; // para reaacao em cadei se o inimigo morrer
} Enemy;


void initTiro(Ship *s) {

	//printf("\nInit tiro!");
	s->tiro.x = s->x;
	s->tiro.y = s->y;
	s->tiro.raio = 3;
	s->tiro.cor = s->cor;
	s->tiro.timer = TEMPO_TIRO;
	s->tiro.modo = TIRO_INATIVO;

}

void initGlobals() {
	BKG_COLOR = al_map_rgb(10, 10, 10);
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
	FONT_32 = al_load_font("arial.ttf", 32, 1);   
}




void initHero(Hero *s) {
	s->score = 0;
	s->ship.cor = al_map_rgb(100 + rand()%156, 100 + rand()%156, 100 + rand()%156);
	s->ship.x = SCREEN_W/2;
	s->ship.y = SCREEN_H - HERO_H - 10;
	s->ship.vel = 2;
	s->dir_x = 0;
	s->dir_y = 0;
	initTiro(&s->ship);

}

void drawScenario(Hero s) {

	char score_txt[5];

	al_clear_to_color(BKG_COLOR);


}

void drawHero(Hero s) {

	al_draw_filled_triangle(s.ship.x, s.ship.y, s.ship.x - HERO_W/2, s.ship.y + HERO_H, s.ship.x + HERO_W/2, s.ship.y + HERO_H, s.ship.cor);

	char score_txt[5];
	sprintf(score_txt, "%d", (int)s.score);
	//imprime o texto armazenado em my_text na posicao x=10,y=10 e com a cor rgb(5, 102, 10)
	al_draw_text(FONT_32, s.ship.cor, 100, 20, 0, score_txt);	

	al_draw_circle(s.ship.tiro.x, s.ship.tiro.y, s.ship.tiro.raio, s.ship.tiro.cor, BORDA_TIRO);


}





void updateHero(Hero *s) {

	s->ship.x += s->dir_x * s->ship.vel;

	s->ship.y += s->dir_y * s->ship.vel;

	//bloaquear de sair da tela inicio CENTRO DO HEROI É A REFERENCIA
	if (s->ship.x < HERO_W / 2) { //não sai para a esquerda
        s->ship.x = HERO_W / 2; //força voltar pra 15
    }
    if (s->ship.x > SCREEN_W - (HERO_W / 2)) { //não sai para a direita
        s->ship.x = SCREEN_W - (HERO_W / 2); //força voltar para 945
    }

    if (s->ship.y < 0) { //topo da tela 
        s->ship.y = 0;
    }
    if (s->ship.y > SCREEN_H - HERO_H) { 
        s->ship.y = SCREEN_H - HERO_H;
    }// bloquear de sair da tela fim

	if(s->ship.tiro.modo != TIRO_ATIVO) {
		s->ship.tiro.x = s->ship.x;
		s->ship.tiro.y = s->ship.y;
	}
	else {
		if(s->ship.tiro.timer > 0)
			s->ship.tiro.timer -= 1.0/FPS;
		else
			initTiro(&s->ship);
	}
	//diminuir pontuação do jogador

	float penalidade = SCORE_PENALTY * dificuldade; 

    s->score = s->score - (penalidade / FPS);
	if(s->score < 0) {
		s->score = 0; 
	}
}

int vecolisao(float x1, float y1, float raio1, float x2, float y2, float raio2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    float distancia = sqrt(dx * dx + dy * dy);
	if(distancia < (raio1 + raio2)) {
		return 1;
	}
	else {
		return 0;
	}
}


void desenharInimigos(Enemy *enemies) {
	for(int i=0; i<NUM_ENEMIES; i++) {
		if(enemies[i].active)
			al_draw_filled_circle(enemies[i].ship.x, enemies[i].ship.y, enemies[i].raio, enemies[i].ship.cor);
	}

}
void gerarInimigos(Enemy *enemies) {
	for(int i=0; i<NUM_ENEMIES; i++) {
		enemies[i].raio = 20 + rand()%30;
		enemies[i].active = 1;
		enemies[i].ship.vel = 1 + (rand() % 3);
		enemies[i].ship.cor = al_map_rgb(100 + rand()%156, 100 + rand()%156, 100 + rand()%156);
		enemies[i].ship.x = rand()%(SCREEN_W - (int)enemies[i].raio*2) + enemies[i].raio;
		enemies[i].ship.y = -(rand() % 800 + 50); //posicao inicial aleatoria acima da tela e desce com a velocidade programada, assim n desce tudo de uma vez
		enemies[i].explosao.modo = TIRO_INATIVO; //da explosao em cadeia
	}

}
void atualizarInimigos(Enemy *enemies) {
	
	for(int i=0; i<NUM_ENEMIES; i++) {
		if(enemies[i].active) {
			float velocidade = (enemies[i].ship.vel * 0.4) * dificuldade;
			enemies[i].ship.y += velocidade;
		}
		if(enemies[i].ship.y > SCREEN_H + enemies[i].raio) { //sair volta por cima
                enemies[i].ship.x = rand() % (SCREEN_W - (int)enemies[i].raio * 2) + enemies[i].raio;
				enemies[i].ship.y = -(rand() % 400 + 50); //aleatoria acima da tela e desce com a velocidade programada
            }
	}

}
//mata inimigo 

void mata (Hero *heroi, Enemy *enemies, int numInimi) {
	al_play_sample(pontos, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    enemies[numInimi].active = 0; 
    heroi->score += (int)enemies[numInimi].raio; 
    
   //corpo vira dano
    enemies[numInimi].explosao.x = enemies[numInimi].ship.x;
    enemies[numInimi].explosao.y = enemies[numInimi].ship.y;
    enemies[numInimi].explosao.raio = enemies[numInimi].raio; // Fixo no tamanho original
    enemies[numInimi].explosao.cor = enemies[numInimi].ship.cor; 
    enemies[numInimi].explosao.timer = 0.7; // Duração da área de dano
    enemies[numInimi].explosao.modo = TIRO_ATIVO;
    
	//revive no topo
    enemies[numInimi].ship.x = rand() % (SCREEN_W - (int)enemies[numInimi].raio * 2) + enemies[numInimi].raio; 
    enemies[numInimi].ship.y = -(rand() % 400 + 50);
    enemies[numInimi].active = 1; 
}

void colisaoComCampo(Hero *h, Enemy *enemies) {
    if(h->ship.tiro.modo != TIRO_ATIVO) return;

    for(int i = 0; i < NUM_ENEMIES; i++) {
        if(enemies[i].active) {
            int colisao = vecolisao(h->ship.tiro.x, h->ship.tiro.y, h->ship.tiro.raio, enemies[i].ship.x, enemies[i].ship.y, enemies[i].raio);
            
            if(colisao) {
                // chama funcao pra matar
                mata(h, enemies, i);
            }
        }
    }
}



//vai dar trabalhao

void reacaoCadeia(Hero *heroi, Enemy *enemies) {
    for(int i = 0; i < NUM_ENEMIES; i++) {
        if(enemies[i].explosao.modo == TIRO_ATIVO) {
        
            al_draw_circle(enemies[i].explosao.x, enemies[i].explosao.y, enemies[i].explosao.raio, enemies[i].explosao.cor, 2); //desenha area dano
   
            enemies[i].explosao.timer = enemies[i].explosao.timer - 1.0/ FPS;
            if(enemies[i].explosao.timer <= 0) {
                enemies[i].explosao.modo = TIRO_INATIVO;
            }

  
            for(int j = 0; j < NUM_ENEMIES; j++) {
                if(enemies[j].active && i != j) {
                    int colisao = vecolisao(enemies[i].explosao.x, enemies[i].explosao.y, enemies[i].explosao.raio, enemies[j].ship.x, enemies[j].ship.y, enemies[j].raio);
                    if(colisao) {
                
                       mata(heroi, enemies, j);
                    }
                }
            }
        }
    }
}


int pontuacaoRecorde(float pontuacao) {
    FILE *file = fopen("recorde.txt", "r");
    int recordeAtual = 0;
    if(file != NULL) {
        fscanf(file, "%d", &recordeAtual);
        fclose(file);
    }
    if(pontuacao > recordeAtual) {
        file = fopen("recorde.txt", "w");
        if(file != NULL) {
            fprintf(file, "%f", pontuacao);
            fclose(file);
			return 1;
        }
    }
	return 0;
}


int heroiMorre(Hero *h, Enemy *enemies) {
	float centroX = h->ship.x;
    float centroY = h->ship.y + HERO_H / 2.0;
    for(int i = 0; i <NUM_ENEMIES; i++) {
        if(enemies[i].active) {
            int colisao = vecolisao(centroX, centroY, HERO_W/2, enemies[i].ship.x, enemies[i].ship.y, enemies[i].raio);
            if(colisao) {
                return 0; //heroimpeereu
            }
        }
    }
    return 1;
}
 
int main(int argc, char **argv){
	srand(time(NULL));

	int i;
	
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
   
	//----------------------- rotinas de inicializacao ---------------------------------------
    
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
   
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}

	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}
	al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(10);
    pontos= al_load_sample("musicas/pontos.wav");
	gameover = al_load_sample("musicas/gameover.wav");

    fundo = al_load_audio_stream("musicas/fundo.wav", 4, 2048);
	al_attach_audio_stream_to_mixer(fundo, al_get_default_mixer());
    al_set_audio_stream_playmode(fundo, ALLEGRO_PLAYMODE_LOOP);
    al_set_audio_stream_gain(fundo, 0.6);


	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());


	//----------------------jogo-------------------

	//inicializa globais
	initGlobals();

	//Cria o heroi
	Hero Hero;
	initHero(&Hero);


	//cria os inimigos:
	//vetor de inimigos 
	Enemy Enemies[NUM_ENEMIES];
	gerarInimigos(Enemies); //chama a função de gerar os inimigos

	//inicia o temporizador
	al_start_timer(timer);
	
	int playing = 1;

	
	while(playing) {
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			tempoJogo += 1.0 / FPS;
			dificuldade = 1.0 + (tempoJogo * 0.05);// 5% mais dificil, por segundo imigo cai mais rapido e a penalidade
			drawScenario(Hero);
			updateHero(&Hero);
			drawHero(Hero);

			colisaoComCampo(&Hero, Enemies);
			reacaoCadeia(&Hero, Enemies); 

			desenharInimigos(Enemies);
			atualizarInimigos(Enemies);

			//atualiza a tela (quando houver algo para mostrar)

			al_flip_display();
	
			playing = heroiMorre(&Hero, Enemies);

			//pausa o jogo por 3 segundos se o jogador morrer
			if(!playing){
			al_set_audio_stream_playing(fundo, false);
			al_play_sample(gameover, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
			al_rest(3);
			}

		}
		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			//imprime qual tecla foi
			//printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			switch(ev.keyboard.keycode) {
			//se a tecla for o W
				case ALLEGRO_KEY_W:
					Hero.dir_y--;
				break;

				case ALLEGRO_KEY_S:
					Hero.dir_y++;
				break;

				case ALLEGRO_KEY_A:
					Hero.dir_x--;
				break;

				case ALLEGRO_KEY_D:
					Hero.dir_x++;
				break;	

				case ALLEGRO_KEY_SPACE:
					if(Hero.ship.tiro.modo == TIRO_INATIVO) {
						Hero.ship.tiro.modo = TIRO_ATIVO;
						Hero.ship.tiro.raio = RAIO_TIRO;
					}
				break;
			}
		}			

		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			//imprime qual tecla foi
			//printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			switch(ev.keyboard.keycode) {
			//se a tecla for o W
				case ALLEGRO_KEY_W:
					Hero.dir_y++;
				break;

				case ALLEGRO_KEY_S:
					Hero.dir_y--;
				break;

				case ALLEGRO_KEY_A:
					Hero.dir_x++;
				break;

				case ALLEGRO_KEY_D:
					Hero.dir_x--;
				break;	
				/*
				case ALLEGRO_KEY_SPACE:
					if(Hero.tiro.modo == TIRO_HOLDING)
						Hero.tiro.modo = TIRO_ATIVO;
				break;		
				*/		
			}			

		}

	} //fim do while
     
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	int bateu = pontuacaoRecorde(Hero.score);

		char my_text[100];
		al_clear_to_color(al_map_rgb(0,0,0));
	 	sprintf(my_text, "Pontuação: %d", (int)Hero.score);
		al_draw_text(FONT_32, al_map_rgb(220, 30, 0), SCREEN_W/3, SCREEN_H/2 + 50, 0, my_text);

		if(bateu) {
        al_draw_text(FONT_32, al_map_rgb(0, 255, 0), SCREEN_W/3, SCREEN_H/2 , 0, "NOVO RECORDE!");
        }

		al_flip_display();
		al_rest(3);	
 
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_sample(pontos);
	al_destroy_sample(gameover);
	al_destroy_audio_stream(fundo);
	return 0;
}