/*-------------------------------------------------------------*/
/*                                                             */
/*-------------------------------------------------------------*/

/** INITIALISATION DES MODULES **/
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*-------------------------------------------------------------*/
/*                                                             */
/*-------------------------------------------------------------*/

/** DECLARATIONS DES FONCTONS DE GESTION DU TERMINAL **/
void color(int pair, int cT, int cF);
char nb_getch();
char b_getch();
void startscr();

/*-------------------------------------------------------------*/
/*                                                             */
/*-------------------------------------------------------------*/

/** DECLARATION DES FONCTONS DE GESTION DU JEU **/
void loadStart();
void stars();
void afficherMenu(int choix);
void rules();
void modif(char *player[3]);
void map(int score);
void niveau(int stage, int enemysPos[10][2], int miniBoss[2][8], int boss[8], int x);
int vaisseau(char *Player[3], int x);
void updateEnemys(int enemysPos[10][2], int *varEnemy);
void updateLasers(int lasers[2][4], int enemysPos[10][2], int *score, int miniBoss[2][8], int boss[8], int x, char *Player[3]);
void updateMiniBoss(int miniBoss[2][8], int enemysPos[10][2], int x);
void updateBoss(int boss[8], int x);
void deleteRow(int x);
void vitesseLumiere(int x, char *Player[3], int boss[8]);

/*-------------------------------------------------------------*/
/*                                                             */
/*-------------------------------------------------------------*/

/** DECLARATION DES CONSTANTES **/
const int MIN_Y = 5, MAX_Y = 30, MIN_X = 9, MAX_X = 60;

/** DECLARATION DES VARIABLES GENERALES **/
bool again = true, paused = false, defeat = false, win = false;

/** HIGHSCORE **/
FILE *fp;
char *filename = "highScore.txt";

/*-------------------------------------------------------------*/
/*                                                             */
/*-------------------------------------------------------------*/

/** FONCTION PRINCIPALE **/
int main() {
  /* Variables menu / jeu */
  int choix = 1, score = 0;

  /* Variables joueur */
  char *Player[3] = {"| A |", "!=H=!", " * * "};
  int x = 35;

  /* Variables enemis */
  int enemysPos[10][2] =
      {/* { y, x }, */
       {-1, MIN_X + 2},  {-1, MIN_X + 7},  {-1, MIN_X + 12}, {-1, MIN_X + 17},
       {-1, MIN_X + 22}, {-1, MIN_X + 27}, {-1, MIN_X + 32}, {-1, MIN_X + 37},
       {-1, MIN_X + 42}, {-1, MIN_X + 47}},
      varEnemy = 0;

  /* Variables mini boss */
  int miniBoss[2][8] = {
    // { y, x, vies, varMiniBoss, yLaser, xLaser, varLaser, binLaser },
    { -1, -1, 3, 0, 0, 0, 0, 0 },
    { -1, -1, 3, 0, 0, 0, 0, 0 }
  };

  /* Variable boss */
  int boss[8] = { -1, -1, 0, 0, 0, 0, 0, 6 }; // { y, x, varLaser, binLaser, varLoad, binLoad, varUnload, vies };

  /* Variables laser */
  int lasers[2][4] = {/* { y, x, varLaser, binLaser } */
                      {0, 0, 0, 0},
                      {0, 0, 0, 0}};

  /* Variables génération aléatoire */
  time_t t;
  srand((unsigned)time(&t));

  /*-------------------------------------------------------------*/
  /*                                                             */
  /*-------------------------------------------------------------*/

  /* Les instructions avant le début du jeu */
  startscr();

  /* Affichage de l'écran d'accueil */
  // vitesseLumiere(x, Player);
  loadStart();

  /* Affichage des menus */
  afficherMenu(choix);
  while (again) {
    switch (getch()) {
    case KEY_DOWN:
      (choix == 4) ? choix = 1 : choix++;
      afficherMenu(choix);
      break;
    case KEY_UP:
      (choix == 1) ? choix = 4 : choix--;
      afficherMenu(choix);
      break;
    case '\n':
    case ' ':
      if (choix == 1 || choix == 4)
        again = false;
      if (choix == 3)
        modif(Player);
      if (choix == 2)
        rules();
      afficherMenu(choix);
      break;
    }
  }
  clear();

  /*-------------------------------------------------------------*/
  /*                                                             */
  /*-------------------------------------------------------------*/
  /* Initilisation du niveau */
  map(score);
  niveau(1, enemysPos, miniBoss, boss, x);

  /* Dans la boucle while les instruction pour le jeu */
  while (choix == 1) {
    if (!paused) {
      /* Affichages */
      map(score);

      switch (nb_getch()) {
      case 'd':
      case 'D':
        if (x < MAX_X - 7)
          x++;
        break;
      case 'q':
      case 'Q':
        if (x > MIN_X + 3)
          x--;
        break;
      case ' ':
        if (lasers[0][3] != 1 || lasers[1][3] != 1) {
          /* Update y and state */
          for (int i = 0; i < 2; i++) {
            lasers[i][0] = MAX_Y - 1;
            lasers[i][3] = 1;
          }
          /* Update x */
          lasers[0][1] = x;
          lasers[1][1] = x + 4;
        }
        break;
      case 'p':
      case 'P':
        paused = true;
        move(MIN_Y + 3, MAX_X + 11);
        color(6, COLOR_RED, COLOR_BLACK);
        printw("STOPPED");
        break;
      }

      /* Gestion du vaisseau */
      x = vaisseau(Player, x);

      /* Gestion des méchants */
      updateEnemys(enemysPos, &varEnemy);
      /* Gestion des lasers */
      updateLasers(lasers, enemysPos, &score, miniBoss, boss, x, Player);
      /* Gestion des miniBoss */
      updateMiniBoss(miniBoss, enemysPos, x);
      /* Gestion du boss */
      updateBoss(boss, x);

      refresh();
      usleep(5000);
    } else {
      if (!defeat) {
        if(!win){
          switch (b_getch()) {
          case 'p':
          case 'P':
            paused = false;
            break;
          }
        }else{
          /* Ecran de victoire */

          // Textes (crédis / score) 
          clear();
          
          move(5, 0);
          color(1, COLOR_YELLOW, COLOR_BLACK);
          printw("\t     _____\n");
          printw("\t    / ____|\n");
          printw("\t   | (___  _ __   __ _  ___ ___\n");
          printw("\t    \\___ \\| '_ \\ / _` |/ __/ _ \\\n");
          printw("\t    ____) | |_) | (_| | (_|  __/\n");
          printw("\t   |_____/| .__/ \\__,_|\\___\\___|\n");
          printw("\t   |_   _|| |                | |\n");
          printw("\t     | |  |_|___   ____ _  __| | ___ _ __ \n");
          printw("\t     | | | '_ \\ \\ / / _` |/ _` |/ _ \\ '__|\n");
          printw("\t    _| |_| | | \\ V / (_| | (_| |  __/ |\n");
          printw("\t   |_____|_| |_|\\_/ \\__,_|\\__,_|\\___|_|");
        
          color(3, COLOR_WHITE, COLOR_BLACK);
          printw("\n\n\n\t\t\tScore : ");
          color(1, COLOR_YELLOW, COLOR_BLACK);
          printw("%d", score);
          
          color(3, COLOR_WHITE, COLOR_BLACK);      
          printw("\n\t\t   Meilleur Score : ");
          color(1, COLOR_YELLOW, COLOR_BLACK);
          fp = fopen(filename, "r");
          char ligne[256];
          while (fgets(ligne, sizeof(ligne), fp) != NULL) {
            int highScore = atoi(ligne);
            if(highScore < score){
              fclose(fp);
              fp = fopen(filename, "w");
              fprintf(fp, "%d", score);
              fclose(fp);
              fp = fopen(filename, "r");
            }else printw("%d", highScore);
          }
          fclose(fp);

          
          color(2, 8, COLOR_BLACK);
          printw("\n\n\n\n\n\tAppuyez sur une touche pour quitter le jeu...");
        
          refresh();
                  
          // Sortie
          int wait = b_getch();
          while (wait != '\n')
            wait = b_getch();
          choix = -1;
        }
      } else {
        /* Affichage contours */
        map(score);

        /* Affichage perdu */
        move(MIN_Y + 3, MAX_X + 11);
        color(6, COLOR_RED, COLOR_BLACK);
        printw("GAME OVER");
        move(MIN_Y + 5, MAX_X + 6);
        printw("Press any button");
        move(MIN_Y + 6, MAX_X + 6);
        printw("to close the game");
        refresh();

        int wait = b_getch();
        while (wait != '\n')
          wait = b_getch();
        choix = -1;
      }
    }
  }

  /* Les instructions avant la cloture du programme */
  // b_getch();

  endwin();
  return 0;
}

/*-------------------------------------------------------------*/
/*                                                             */
/*-------------------------------------------------------------*/

/** DEFINITIONS DES FONCTONS DE GESTION DU TERMINAL **/
/*-------------------------------------------------------------*/
/* Choix des couleurs de la police et de la fenetre CONSOLE    */
/*-------------------------------------------------------------*/
void color(int pair, int cT, int cF) {
  init_pair(pair, cT, cF);
  attron(COLOR_PAIR(pair));
}

/*-------------------------------------------------------------*/
/* Retourne la touche pressee sans bloquer le deroulement      */
/*-------------------------------------------------------------*/
char nb_getch() {
  timeout(0);
  return getch();
}

/*-------------------------------------------------------------*/
/* Retourne la touche pressee en bloquant le deroulement       */
/*-------------------------------------------------------------*/
char b_getch() {
  timeout(-1);
  return getch();
}

/*-------------------------------------------------------------*/
/* Initialise l'ecran                                          */
/*-------------------------------------------------------------*/
void startscr() {
  initscr();
  start_color();
  // cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, true);
}

/*-------------------------------------------------------------*/
/*                                                             */
/*-------------------------------------------------------------*/

/** DEFINITION DES FONCTONS DE GESTION DU JEU **/
/*-------------------------------------------------------------*/
/* Charge le jeu                                               */
/*-------------------------------------------------------------*/
void loadStart() {
  char *falcon[21] = {
      "          # # # # # # #",
      "        `--._________.--'",
      "      `.  /._________.\\  .'",
      "    `.     /         \\     .'",
      "   \\  \\     /       \\     /  /",
      "  |          /`---'\\          |",
      "  .=----~~~~~\\     /~~~~~----=.",
      "   [::::::::|  (_)  |::::::::]",
      "  `=----.____/  #  \\____.----='",
      "  |           +---+  \\  _.-~  |",
      "   /  /      \\|   |/ .-~    _.-'",
      "    .~  `=='\\ |   | /   _.-'.  |",
      "     /.~ __\\  |   |  /   ~.|   |",
      "      / .--~  |   |  ~--. \\|   |",
      "       /    __|   |__    \\  /_\\",
      "        /     |   |     \\    _",
      "         //  ||___||  \\\\",
      "          // ||   || \\\\",
      "           //||   ||\\\\",
      "            /_|   |_\\",
      "             _     _",
  };
  /*
             _     _
            /_|   |_\
           //||   ||\\
          // ||   || \\
         //  ||___||  \\
        /     |   |     \    _
       /    __|   |__    \  /_\
      / .--~  |   |  ~--. \|   |
     /.~ __\  |   |  /   ~.|   |
    .~  `=='\ |   | /   _.-'.  |
   /  /      \|   |/ .-~    _.-'
  |           +---+  \  _.-~  |
  `=----.____/  #  \____.----='
   [::::::::|  (_)  |::::::::]
  .=----~~~~~\     /~~~~~----=.
  |          /`---'\          |
   \  \     /       \     /  /
    `.     /         \     .'
      `.  /._________.\  .'
        `--._________.--'
          # # # # # # #
  */

  clear();

  for (int y = MAX_X + 25; y >= 0; y--) {
    move(y, MIN_X + 3);
    for (int i = 0; i < 21; i++) {
      if (i == 0)
        color(2, COLOR_BLUE, COLOR_BLACK);
      else
        color(1, 8, COLOR_BLACK);

      move(y - i, MIN_X + 3);
      if (!(y - i <= 0) && !(y - i >= MAX_Y))
        printw("%s", falcon[i]);
    }

    stars(MAX_Y, MAX_X);
    refresh();

    usleep(50000);
    clear();
  }

  sleep(1);

  /*
   _____
  / ____|
 | (___  _ __   __ _  ___ ___
  \___ \| '_ \ / _` |/ __/ _ \
  ____) | |_) | (_| | (_|  __/
 |_____/| .__/ \__,_|\___\___|
 |_   _|| |                | |
   | |  |_|___   ____ _  __| | ___ _ __
   | | | '_ \ \ / / _` |/ _` |/ _ \ '__|
  _| |_| | | \ V / (_| | (_| |  __/ |
 |_____|_| |_|\_/ \__,_|\__,_|\___|_|
  */
  move(MIN_Y, 0);
  color(1, COLOR_YELLOW, COLOR_BLACK);
  printw("\t     _____\n");
  printw("\t    / ____|\n");
  printw("\t   | (___  _ __   __ _  ___ ___\n");
  printw("\t    \\___ \\| '_ \\ / _` |/ __/ _ \\\n");
  printw("\t    ____) | |_) | (_| | (_|  __/\n");
  printw("\t   |_____/| .__/ \\__,_|\\___\\___|\n");
  printw("\t   |_   _|| |                | |\n");
  printw("\t     | |  |_|___   ____ _  __| | ___ _ __ \n");
  printw("\t     | | | '_ \\ \\ / / _` |/ _` |/ _ \\ '__|\n");
  printw("\t    _| |_| | | \\ V / (_| | (_| |  __/ |\n");
  printw("\t   |_____|_| |_|\\_/ \\__,_|\\__,_|\\___|_|");

  stars(MAX_Y, MAX_X);
  refresh();

  sleep(1);
  move(MAX_Y - 2, MIN_X);
  color(2, 8, COLOR_BLACK);
  printw("\tAppuyer pour commencer...");
  getch();
}

/*-------------------------------------------------------------*/
/* Affiche des étoiles                                         */
/*-------------------------------------------------------------*/
void stars() {
  time_t t;
  srand((unsigned)time(&t));

  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
      if (rand() % 50 == 5) {
        move(y, x);
        color(3, COLOR_WHITE, COLOR_BLACK);
        printw(".");
      }
    }
  }
}

/*-------------------------------------------------------------*/
/* Affiche le menu                                             */
/*-------------------------------------------------------------*/
void afficherMenu(int choix) {
  clear();

  /*
  ___  ___   _    ___  ___
 / __|| _ \ /_\  / __|| __|
 \__ \|  _// _ \| (__ | _|
 |___/|_| /_/ \_\\___||___|

  ___  _  _ __   __ _    ___   ___  ___
 |_ _|| \| |\ \ / //_\  |   \ | __|| _ \
  | | | .` | \ V // _ \ | |) || _| |   /
 |___||_|\_|  \_//_/ \_\|___/ |___||_|_\


          ___  ___   _    ___  ___
         / __|| _ \\ /_\\  / __|| __|
     ____\\__ \\|  _// _ \\| (__ | _|_____________________________
    / _______/|_| /_/ \\_\\\\___||_______________________________ \\
   / /                                                        \\ \\
  | |                                                          | |
  | |                                                          | |
  | |                                                          | |
  | |                                                          | |
  | |                       <Your text>                        | |
  | |                                                          | |
  | |                                                          | |
  | |                                                          | |
  | |                                                          | |
   \\ \\________________  _  _ __   __ _    ___   ___  _________\/ \/
    \\_______________ _|| \\| |\\ \\ / //_\\  |   \\ | __|| _ \\______\/
                   | | | .` | \\ V // _ \\ | |) || _| |   /
                  |___||_|\\_|  \\_//_/ \\_\\|___/ |___||_|_\\

  */

  move(0, 0);
  color(1, COLOR_WHITE, COLOR_BLACK);
  printw(
      "          ___  ___   _    ___  ___\n"
      "         / __|| _ \\ /_\\  / __|| __|\n"
      "     ____\\__ \\|  _// _ \\| (__ | _|____________________________\n"
      "    / _______/|_| /_/ \\_\\\\___||______________________________ \\\n"
      "   / /                                                       \\ \\\n"
      "  | |                                                         | |\n"
      "  | |                                                         | |\n"
      "  | |                                                         | |\n"
      "  | |                                                         | |\n"
      "  | |                                                         | |\n"
      "  | |                                                         | |\n"
      "  | |                                                         | |\n"
      "  | |                                                         | |\n"
      "   \\ \\________________  _  _ __   __ _    ___   ___  ________/ /\n"
      "    \\_______________ _|| \\| |\\ \\ / //_\\  |   \\ | __|| _ \\_____/\n"
      "                   | | | .` | \\ V // _ \\ | |) || _| |   /\n"
      "                  |___||_|\\_|  \\_//_/ \\_\\|___/ |___||_|_\\");

  move(7, 31);
  (choix == 1) ? color(2, COLOR_YELLOW, COLOR_BLACK)
               : color(1, COLOR_WHITE, COLOR_BLACK);
  printw("Jouer");

  move(8, 27);
  (choix == 2) ? color(2, COLOR_YELLOW, COLOR_BLACK)
               : color(1, COLOR_WHITE, COLOR_BLACK);
  printw("Regles du jeu");

  move(9, 25);
  (choix == 3) ? color(2, COLOR_YELLOW, COLOR_BLACK)
               : color(1, COLOR_WHITE, COLOR_BLACK);
  printw("Parametres du jeu");

  move(10, 23);
  (choix == 4) ? color(2, COLOR_YELLOW, COLOR_BLACK)
               : color(1, COLOR_WHITE, COLOR_BLACK);
  printw("Quitter l application");

  refresh();
}

/*-------------------------------------------------------------*/
/* Affiche les règles du jeu                                   */
/*-------------------------------------------------------------*/
void rules() {
  int page = 1;
  bool next = true;

  clear();

  do {
    if (page == 1) {
      /* Page 1 */
      color(4, COLOR_WHITE, COLOR_BLACK);
      printw("\n\n\t\t\tUTILISATION DES TOUCHES");

      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("\n\n\n\n\nLe vaisseau :");
      char *text_1[8] = {
          "\n\tD ou d ->", " Aller vers la droite",
          "\n\tQ ou q ->", " Aller vers la gauche",
          "\n\tespace ->", " Tire des lasers",
          "\n\tP ou p ->", " Met le jeu en pause / quitte la pause"};
      for (int i = 0; i < 8; i++) {
        (i % 2 == 0) ? color(4, COLOR_WHITE, COLOR_BLACK)
                     : color(1, 8, COLOR_BLACK);
        printw("%s", text_1[i]);
      }

      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("\n\n\n\nAutres :");
      char *text_2[4] = {"\n\tFleches directionnelles ->",
                         " Ce deplacer dans le menu,\n\t\t\tles parametres "
                         "ainsi que dans les regles du jeu",
                         "\n\tEchap -> ",
                         "Fonction retour dans les "
                         "parametres\n\t\t\tainsi que dans les regles du jeu"};
      for (int i = 0; i < 4; i++) {
        (i % 2 == 0) ? color(4, COLOR_WHITE, COLOR_BLACK)
                     : color(1, 8, COLOR_BLACK);
        printw("%s", text_2[i]);
      }

      color(4, COLOR_WHITE, COLOR_BLACK);
      move(MAX_Y, MAX_X - 15);
      printw("Introduction au jeu || ->");
    } else if (page == 2) {
      /* Page 2 */
      color(4, COLOR_WHITE, COLOR_BLACK);
      printw("\n\n\t\t\tINTRODUCTION AU JEU");
      printw("\n\n\nLe jeu contient 5 vagues d'ennemis de plus en plus durs :");

      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("\n\n\tVague 1 (debut) : ");
      color(1, 8, COLOR_BLACK);
      printw("\n\t\t3 Vaisseaux.");
      
      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("\n\n\tVague 2 (500 pts) : ");
      color(1, 8, COLOR_BLACK);
      printw("\n\t\t5 Vaisseaux.");
      
      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("\n\n\tVague 3 (1000 pts) : ");
      color(1, 8, COLOR_BLACK);
      printw("\n\t\t5 Vaisseaux, 1 MINI-BOSS.");
      
      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("\n\n\tVague 4 (2500 pts) : ");
      color(1, 8, COLOR_BLACK);
      printw("\n\t\t6 Vaisseaux, 2 MINI-BOSS.");
      
      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("\n\n\tVague 5 (5000 pts) : ");
      color(1, 8, COLOR_BLACK);
      printw("\n\t\t7 Vaisseaux, 2 MINI-BOSS, 1 BIG-BOSS.");
      
      color(4, COLOR_WHITE, COLOR_BLACK);
      printw("\n\n\nL'objectif est de tuer le BIG-BOSS, afin de gagner.");
      
      move(MAX_Y, MIN_X - 9);
      printw("<- || Utilisation des touches");
      move(MAX_Y, MAX_X - 17);
      printw("Pressision sur le jeu || ->");
    }else if(page == 3){
      /* Page 3 */
      color(4, COLOR_WHITE, COLOR_BLACK);
      printw("\n\n\t\t\tPRESSISION SUR LE JEU\n\n\n\n");

      color(1, 8, COLOR_BLACK);
      printw("\n\n\nVotre vaisseau possede ");
      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("qu'une seule ");
      color(1, 8, COLOR_BLACK);
      printw("vie !");


      printw("");
      
      color(1, 8, COLOR_BLACK);
      printw("\n\n\nLes MINI-BOSS possedent ");
      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("3 ");
      color(1, 8, COLOR_BLACK);
      printw("vies, se deplacent sur X et Y et tirent des lasers !");

      
      color(1, 8, COLOR_BLACK);
      printw("\n\n\nLe BIG-BOSS possede ");
      color(3, COLOR_YELLOW, COLOR_BLACK);
      printw("6 ");
      color(1, 8, COLOR_BLACK);
      printw("vies, se deplace que sur X et tire des lasers destructeurs !");

      color(4, COLOR_WHITE, COLOR_BLACK);
      move(MAX_Y, MIN_X - 9);
      printw("<- || Introduction au jeu");
    }

    switch (getch()) {
    case 27:  /* ESCAPE */
    case ' ': /* SPACE */
    case '\n':
      next = false;
      break;
    case KEY_LEFT:
      if(page != 1)
        page--;
      break;
    case KEY_RIGHT:
      if(page != 3)
        page++;
      break;
    }
    clear();
  } while (next);
}

/*-------------------------------------------------------------*/
/* Affiche le contour du jeu                                   */
/*-------------------------------------------------------------*/
void map(int score) {
  /* Couleur */
  color(1, COLOR_WHITE, COLOR_BLACK);

  /* Cadre */
  for (int y = MIN_Y; y < MAX_Y + 1; y++) {
    for (int x = MIN_X; x < MAX_X + 1; x++) {
      move(y, x);
      if (x == MIN_X || x == MAX_X)
        printw("|");
    }
  }

  /* Partie de droite */
  move(MIN_Y, MAX_X + 7);
  printw("Space Invader");
  move(MIN_Y + 2, MAX_X + 2);
  printw("Score : ");
  color(4, COLOR_YELLOW, COLOR_BLACK);
  printw("%d", score);

  color(1, COLOR_WHITE, COLOR_BLACK);
  move(MIN_Y + 3, MAX_X + 2);
  printw("Status : ");
  move(MIN_Y + 3, MAX_X + 11);
  color(6, COLOR_GREEN, COLOR_BLACK);
  printw("RUNNING");
}

/*-------------------------------------------------------------*/
/* Affiche le vaisseau et le déplace avec "q" ou "d"           */
/*-------------------------------------------------------------*/
int vaisseau(char *Player[3], int x) {
  /* Affiche le vaisseau */
  for (int i = 0; i < 3; i++) {
    if (i == 2)
      color(5, COLOR_RED, COLOR_BLACK);
    else
      color(2, 8, COLOR_BLACK);
    move(MAX_Y + i, x);
    printw("%s", Player[i]);
    deleteRow(x);
  }

  /* Renvoit de x */
  return x;
}

/*-------------------------------------------------------------*/
/* Gestion des méchants                                        */
/*-------------------------------------------------------------*/
void updateEnemys(int enemysPos[10][2],
                  int *varEnemy) {
  char *Enemy = "|o|";
  
  /* Affichage des méchants */
  for (int enemy = 0; enemy < 10; enemy++) {
    if (enemysPos[enemy][0] != -1) {
      color(2, 8, COLOR_BLACK);
      move(enemysPos[enemy][0], enemysPos[enemy][1]);
      printw("%s", Enemy);
    }
  }

  /* Update les méchants */
  *varEnemy += 1;
  if (*varEnemy % 150 == 0) {
    for (int enemy = 0; enemy < 10; enemy++) {
      if (enemysPos[enemy][0] != -1) {
        if (enemysPos[enemy][0] == MAX_Y - 1) {
          // Game over
          paused = true;
          defeat = true;
        } else {
          // Déplacement du vaisseau
          enemysPos[enemy][0]++;
          move(enemysPos[enemy][0] - 1, enemysPos[enemy][1]);
        }
        printw("   ");
      }
    }
    *varEnemy = 0;
  }
}

/*-------------------------------------------------------------*/
/* Gestion des lasers                                          */
/*-------------------------------------------------------------*/
void updateLasers(int lasers[2][4], int enemysPos[10][2], int *score, int miniBoss[2][8], int boss[8], int x, char *Player[3]) {
  for (int i = 0; i < 2; i++) {
    // Laser
    lasers[i][2]++;
    if (lasers[i][3] == 1 && (lasers[i][2] % 5 == 0)) {
      color(5, COLOR_RED, COLOR_BLACK);
      move(lasers[i][0], lasers[i][1]);
      printw("|");
      if (lasers[i][0] != MAX_Y - 1) {
        move(lasers[i][0] + 1, lasers[i][1]);
        printw(" ");
      }
      lasers[i][0]--;

      // Collision enemis
      for (int w = 0; w < 10; w++) {
        if ((lasers[i][0] == MIN_Y - 2) ||
            (enemysPos[w][0] == lasers[i][0] - 1 &&
             (enemysPos[w][1] >= lasers[i][1] - 2 &&
              enemysPos[w][1] <= lasers[i][1]))) {
          lasers[i][3] = 0;
          move(lasers[i][0] + 1, lasers[i][1]);
          printw(" ");
          lasers[i][2] = 0;
        }
        if ((enemysPos[w][0] >= lasers[i][0] - 1 && enemysPos[w][0] <= lasers[i][0]) &&
            (enemysPos[w][1] >= lasers[i][1] - 2 &&
             enemysPos[w][1] <= lasers[i][1])) {
          int r = rand() % 10;
          move(enemysPos[w][0], enemysPos[w][1]);
          enemysPos[w][0] = -1;
          printw("   ");
          *score += 50;
          while (enemysPos[r][0] != -1)
            r = rand() % 10;
          enemysPos[r][0] = (rand() % 3) + MIN_Y + 1;

          if (*score == 5000)
            niveau(5, enemysPos, miniBoss, boss, x);
          if (*score == 2500)
            niveau(4, enemysPos, miniBoss, boss, x);
          if (*score == 1000)
            niveau(3, enemysPos, miniBoss, boss, x);
          if (*score == 500)
            niveau(2, enemysPos, miniBoss, boss, x);
        }
      }

      // Collision mini boss
      for(int m = 0; m < 2; m++){
        if ((lasers[i][0] == MIN_Y - 2) ||
            (miniBoss[m][0] == lasers[i][0] - 1 &&
             (miniBoss[m][1] >= lasers[i][1] - 2 &&
              miniBoss[m][1] <= lasers[i][1]))) {
          lasers[i][3] = 0;
          move(lasers[i][0] + 1, lasers[i][1]);
          printw(" ");
          lasers[i][2] = 0;
        }
        if ((miniBoss[m][0] >= lasers[i][0] - 1 && miniBoss[m][0] <= lasers[i][0]) &&
            (miniBoss[m][1] >= lasers[i][1] - 2 &&
             miniBoss[m][1] <= lasers[i][1])) {
          // Si plus de vie
          if(miniBoss[m][2] == 1){
            move(miniBoss[m][0], miniBoss[m][1]);
            miniBoss[m][0] = -1;
            printw("   ");
          }
          miniBoss[m][2]--;
        }
      }

      // Collision boss
      if((lasers[i][1] >= boss[1] && lasers[i][1] <= boss[1] + 12) && 
         (lasers[i][0] == MIN_Y - 2)){
        if(boss[7] == 1){
          // Fin
          paused = true;
          vitesseLumiere(x, Player, boss);
        }
        boss[7]--;
      }
    }
  }
}

/*-------------------------------------------------------------*/
/* Supprime le caractère à gauche ainsi qu'a droite            */
/*-------------------------------------------------------------*/
void deleteRow(int x) {
  /* Suppression droite */
  move(MAX_Y + 1, x + 5);
  printw("  ");
  move(MAX_Y, x + 5);
  printw("  ");

  /* Suppression gauche */
  move(MAX_Y + 1, x - 2);
  printw("  ");
  move(MAX_Y, x - 2);
  printw("  ");
}

/*-------------------------------------------------------------*/
/* Affiche les paramètres du jeu                               */
/*-------------------------------------------------------------*/
void modif(char *Player[3]) {
  char *Vaisseaux[3][3] = {{"| A |", "!=H=!", " * * "},
                           {"/| |\\", "\\_O_/", " * * "},
                           {"|\\_/|", "\\_H_/", " * * "}};


  int touch, selec = 36;

  clear();
  
  do {
    // Texte
    move(MIN_Y, MIN_X);
    color(1, COLOR_WHITE, COLOR_BLACK);
    printw("   Appuyez sur les fleches ");
    color(7, COLOR_YELLOW, COLOR_BLACK);
    printw("GAUCHE");
    color(1, COLOR_WHITE, COLOR_BLACK);
    printw(" ou ");
    color(7, COLOR_YELLOW, COLOR_BLACK);
    printw("DROITE");
    color(1, COLOR_WHITE, COLOR_BLACK);
    printw("\n\t\t  afin de changer de vaisseau.");

    printw("\n\n\t Pressez ensuite ");
    color(7, COLOR_YELLOW, COLOR_BLACK);
    printw("ENTRER");
    color(1, COLOR_WHITE, COLOR_BLACK);
    printw(" afin de le selectionner.");
    
    // Vaisseaux
    for (int vaisseau = 0; vaisseau < 3; vaisseau++) {
      for (int i = 0; i < 3; i++) {
        move(16 + i, 18 * (vaisseau + 1) - 4);
        if (i == 2)
          color(5, COLOR_RED, COLOR_BLACK);
        else
          color(2, 8, COLOR_BLACK);
        printw("%s", Vaisseaux[vaisseau][i]);
      }
    }

    // Cadre
    color(7, COLOR_YELLOW, COLOR_BLACK);
    for (int x = selec - 7; x < selec + 4; x++) {
      move(19, x);
      if (x > selec - 6 && x < selec + 2)
        printw("_");
      if (x == selec - 6)
        printw("\\");
      if (x == selec + 2)
        printw("/");
    }

    refresh();

    // Touches
    touch = getch();
    switch (touch) {
    case KEY_LEFT:
      if (selec != 18)
        selec -= 18;
      else selec = 54;
      clear();
      break;
    case KEY_RIGHT:
      if (selec != 54)
        selec += 18;
      else selec = 18;
      clear();
      break;
    }
  } while (touch != '\n');

  selec = (selec / 3 / 3 / 3);

  for (int i = 0; i < 3; i++) {
    Player[i] = Vaisseaux[selec][i];
  }
}

/*-------------------------------------------------------------*/
/* Gestion des niveaux (stage)                                 */
/*-------------------------------------------------------------*/
void niveau(int stage, int enemysPos[10][2], int miniBoss[2][8], int boss[8], int x) {
  int vaisseaux = 0, miniboss = 0, nBoss = 0;
  switch (stage) {
  case 1:
    vaisseaux = 3;
    break;
  case 2:
    vaisseaux = 2;
    break;
  case 3:
    vaisseaux = 0;
    miniboss = 1;
    break;
  case 4:
    vaisseaux = 1;
    miniboss = 2;
    break;
  case 5:
    vaisseaux = 1;
    miniboss = 2;
    nBoss = 1;
    break;
  }

  for (int i = 0; i < vaisseaux; i++) {
    int ennemy = rand() % 10;
    while (enemysPos[ennemy][0] != -1)
      ennemy = rand() % 10;
    enemysPos[ennemy][0] = (rand() % 3) + MIN_Y + 1;
  }

  for(int i = 0; i < miniboss; i++){
    int mini = rand() % 2;
    while(miniBoss[mini][0] != -1) 
      mini = rand() % 2;
    miniBoss[mini][0] = MIN_Y;
    (i == 0) ? (miniBoss[mini][1] = (rand() % 20) + MIN_X) : (miniBoss[mini][1] = (rand() % 20) + MIN_X + 25);

    miniBoss[mini][2] = 3;
  }

  for(int i = 0; i < nBoss; i++){
    // Affichage du boss
    char deathStar[5][15] = {
      "  ,_~\"\"\"~-,  ",
      ".'(_)------`,",
      "|===========|", 
      "`,---------,'",
      "  ~-.___.-~  "
    };
    for(int y = -5; y <= 1; y++){
      color(2, 8, COLOR_BLACK);
      move(y, x - 2);
      for(int l = 0; l < 5; l++){
        move(y + l, x - 2);
        if (!(y + l <= 0) && !(y + l >= 6))
          printw("%s", deathStar[l]);
      }
      refresh();
      usleep(500000);
      clrtoeol();
    }

    // Initialisation du boss
    boss[1] = x - 2;
  }
}

/*-------------------------------------------------------------*/
/* Gestion des mini boss                                       */
/*-------------------------------------------------------------*/
void updateMiniBoss(int miniBoss[2][8], int enemysPos[10][2], int x){
  char *MiniBoss = "{o}";
  
  for(int mBoss = 0; mBoss < 2; mBoss++){
    
    // Affichage des miniBoss
    if(miniBoss[mBoss][0] != -1){
      color(2, 8, COLOR_BLACK);
      move(miniBoss[mBoss][0], miniBoss[mBoss][1]);
      printw("%s", MiniBoss);

      // Update des miniBoss (y et x)
      miniBoss[mBoss][3]++;
      if(miniBoss[mBoss][3] % 150 == 0){
        int rnd = rand() % 10;
        bool moveable = true;
  
        // Test de mouvement
        for(int enemys = 0; enemys < 10; enemys++){
          if(enemysPos[enemys][0] != -1){
            // Ne peut pas descendre
            if(enemysPos[enemys][0] == miniBoss[mBoss][0] + 1) moveable = false;
            // Ne peux pas aller à droite
            if(enemysPos[enemys][1] == miniBoss[mBoss][1] + 3) moveable = false;
            // Ne peux pas aller à gauche
            if(enemysPos[enemys][1] + 3 == miniBoss[mBoss][1]) moveable = false;
          }
        }
  
        if(moveable){
          if(miniBoss[mBoss][0] == MAX_Y - 1){
            // Game over
            defeat = true;
            paused = true;
          }else{
            // Déplacement du miniBoss
            switch(rnd){
              case 0: // Gauche
              case 2:
                if(miniBoss[mBoss][1] > MIN_X + 3)
                  miniBoss[mBoss][1]--;
                break;
              case 1: // Droite
              case 3:
                if(miniBoss[mBoss][1] < MAX_Y - 5)
                  miniBoss[mBoss][1]++;
                break;
              case 9: // Laser
                if(miniBoss[mBoss][7] == 0){
                  miniBoss[mBoss][7] = 1;
                  miniBoss[mBoss][4] = miniBoss[mBoss][0] + 1;
                  miniBoss[mBoss][5] = miniBoss[mBoss][1] + 1;
                }
                break;
            }
            miniBoss[mBoss][0]++;
            move(miniBoss[mBoss][0] - 1, miniBoss[mBoss][1] - 1);
          }
          printw("     ");
        }
        miniBoss[mBoss][3] = 0;
      }
  
      
      // Update des lasers des miniBoss
      miniBoss[mBoss][6]++;
      if(miniBoss[mBoss][7] == 1 && miniBoss[mBoss][6] % 5 == 0){
        color(9, COLOR_GREEN, COLOR_BLACK);
        move(miniBoss[mBoss][4], miniBoss[mBoss][5]);
        printw("|");
        if(miniBoss[mBoss][4] != miniBoss[mBoss][0] - 1){
          move(miniBoss[mBoss][4] - 1, miniBoss[mBoss][5]);
          printw(" ");
        }
        miniBoss[mBoss][4]++;
  
        if(miniBoss[mBoss][4] == MAX_Y){
          if(miniBoss[mBoss][5] >= x && miniBoss[mBoss][5] <= x + 4){
            // Game over
            defeat = true;
            paused = true;
          }else{
            // Pas touché
            miniBoss[mBoss][7] = 0;
            miniBoss[mBoss][6] = 0;
            move(miniBoss[mBoss][4] - 1, miniBoss[mBoss][5]);
            printw(" ");
          }
        }
      }
    }
  }
}
  
/*-------------------------------------------------------------*/
/* Gestion du boss final                                       */
/*-------------------------------------------------------------*/
void updateBoss(int boss[8], int x){
  char deathStar[5][15] = {
    "  ,_~\"\"\"~-,  ",
    ".'(_)------`,",
    "|===========|", 
    "`,---------,'",
    "  ~-.___.-~  "
  };
  
  // Si boss sur le terrain
  if(boss[1] != -1){
    if(boss[3] == 0){
      if((boss[1] > MIN_X + 6) && (boss[1] < MAX_X - 18)){
        // Update du boss (mouvement)
        for(int l = 0; l < 5; l++){
          color(2, 8, COLOR_BLACK);
          move(l + 1, x - 3);
          printw("                ");
          move(l + 1, x - 2);
          printw("%s", deathStar[l]);
        }
        boss[1] = x - 2;
      }else boss[1] = x - 1;
    }
    

    // Laser du boss
    boss[2]++;
    if((boss[2] % 2000 == 0) || boss[3] == 1){
      boss[3] = 1;
      boss[4]++;
      
      color(9, COLOR_GREEN, COLOR_BLACK);
      move(2, boss[1] + 3);
      printw("X");

      if((boss[4] % 500 == 0) || boss[5] == 1){
        boss[5] = 1;
        boss[6]++;
        for(int y = 3; y < MAX_Y + 10; y++){
          move(y, boss[1] + 2);
          printw("|||");
        }

        if(((x >= boss[1] + 2) && (x <= boss[1] + 4)) || ((x + 4 >= boss[1] + 2) && (x + 4 <= boss[1] + 4))){
          // Game over
          defeat = true;
          paused = true;
        }

        if(boss[6] % 500 == 0){
          for(int y = 2; y < MAX_Y + 10; y++){
            move(y, boss[1] + 2);
            printw("   ");
          }
          
          boss[2] = 0;
          boss[3] = 0;
          boss[4] = 0;
        }
      }
    }
  }
}
  
/*-------------------------------------------------------------*/
/* Gestion de l'animation de la fin                            */
/*-------------------------------------------------------------*/
void vitesseLumiere(int x, char *Player[3], int boss[8]){
  char deadStar[6][20] = {
    "  ,_~\"\"\" -,  ",
    ".' _)- ---`,",
    "             ",
    "|== ==== ===|", 
    " ,---- ----,'",
    "  ~-.  _.-   "
  };
  Player[2] = " # # ";

  clear();
  
  // Affichage étoile de la mort détruite
  color(2, 8, COLOR_BLACK);
  for(int i = 0; i < 7; i++){
    move(i + 1, x - 2);
    printw("%s", deadStar[i]);
  }

  // Disparition en vitesse lumière
  sleep(2);
  for(int dist = MAX_Y; dist > MIN_Y; dist--){
    // Vaisseau
    for(int i = 0; i < 3; i++){
      if (i == 2)
        color(5, COLOR_BLUE, COLOR_BLACK);
      else
        color(2, 8, COLOR_BLACK);
      move(dist + i, x);
      
      if (!(dist + i <= MIN_Y + 4)){
        printw("%s", Player[i]);
      }
    }

    refresh();
    usleep(10000);
    
    move(dist + 3, x);
    printw("     ");
  }

  // Fin
  sleep(2);
  win = true;
} 
