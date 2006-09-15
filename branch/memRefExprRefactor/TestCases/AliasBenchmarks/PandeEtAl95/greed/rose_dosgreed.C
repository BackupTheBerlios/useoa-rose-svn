// the DOS version of GREED
#include "dosgreed.hpp"
// greed member function definitions

extern "C" { char *strcpy(char *str1,char *str2)
{
  return "";
}

 }

extern "C" { char *strcat(char *str1,char *str2)
{
  return "";
}

 }
static char sBuffer[4096];
// greed class definitions
// **********************************************************************

static void code(char *buffer,int size)
{
  for (size--; size >= 0; size--) {
    buffer[size] ^= 0x5A;
  }
}

// **********************************************************************

uchar Greed::GetPlayTable(uint X,uint Y)
{
  return (((this) -> cPlayTable)[(Y * ((this) -> nMaxX)) + X]);
}

// **********************************************************************

uchar Greed::SetPlayTable(uint X,uint Y,uchar Value)
{
  return (((this) -> cPlayTable)[(Y * ((this) -> nMaxX)) + X] = (Value));
}

// **********************************************************************

int Greed::GetCurrentScore()
{
  return ((this) -> nScore);
}

// **********************************************************************

int Greed::GetSeed()
{
  return (this) -> iSeed;
}

// **********************************************************************

int Greed::IsHighScore()
{
// returns MAX_SCORE if not a high score
// else returns position in the score table
  for (int i = (MAX_SCORE - 1); i >= 0; i--) {
    if (((this) -> nScore) <= (((this) -> ScoreCard)[i].nScore)) {
      return i + 1;
    }
    else {
    }
  }
  return 0;
// end of IsHighScore
}

// **********************************************************************

void Greed::InsertHighScore(char *pszName)
{
  int pos;
// check if really a high score
  pos = (this) -> IsHighScore();
  if (pos == MAX_SCORE) {
    return ;
  }
  else {
  }
// insert the new score displacing the last one
  for (int i = (MAX_SCORE - 1); i > pos; i--) {
    strcpy((((this) -> ScoreCard)[i].szName),(((this) -> ScoreCard)[i - 1].szName));
    ((this) -> ScoreCard)[i].nScore = (((this) -> ScoreCard)[i - 1].nScore);
    ((this) -> ScoreCard)[i].iSeed = (((this) -> ScoreCard)[i - 1].iSeed);
  }
// copy the new name
  ((this) -> ScoreCard)[pos].nScore = ((this) -> nScore);
  ((this) -> ScoreCard)[pos].iSeed = ((this) -> iSeed);
  strcpy((((this) -> ScoreCard)[pos].szName),pszName);
// encode the score file
  strcpy(sBuffer,("game"));
  code(sBuffer,(((sizeof(Score )) * (MAX_SCORE))));
// save the file
// WriteFile (szScoreFile, sBuffer, SCORE_CARD_SIZE);
// end of InsetHighScore
}

// **********************************************************************

Score *Greed::GetHighScore(int Pos)
{
  if ((Pos >= 0)) {
    if ((Pos < MAX_SCORE)) {
      return ((this) -> ScoreCard) + Pos;
    }
    else {
      return ((this) -> ScoreCard) + 0;
    }
  }
  else {{
      return ((this) -> ScoreCard) + 0;
    }
  }
// end of GetHighScore
}

// **********************************************************************

int Greed::AddScore(int nDeltaX,int nDeltaY)
{
// calculates the score to be added depending upon the direction of move
// returns number of moves to make; 0 if no move possible
  int nNewXPos;
  int nNewYPos;
  nNewXPos = ((((this) -> nXPos) + (nDeltaX)));
  nNewYPos = ((((this) -> nYPos) + (nDeltaY)));
  bool rose_sc_bool_0 = false;
  if ((nNewXPos < 0)) {{
      rose_sc_bool_0 = true;
    }
  }
  else {
    if (((nNewXPos) >= ((this) -> nMaxX))) {
      rose_sc_bool_0 = true;
    }
    else {
    }
  }
// is X out of range
  if (rose_sc_bool_0) {
    return 0;
  }
  else {
  }
  bool rose_sc_bool_1 = false;
  if ((nNewYPos < 0)) {{
      rose_sc_bool_1 = true;
    }
  }
  else {
    if (((nNewYPos) >= ((this) -> nMaxY))) {
      rose_sc_bool_1 = true;
    }
    else {
    }
  }
// is Y out of range
  if (rose_sc_bool_1) {
    return 0;
  }
  else {
  }
  uchar ucScoreInc = (this) -> GetPlayTable((nNewXPos),(nNewYPos));
// is it an illegal position
  if ((ucScoreInc) == ('.')) {
    return 0;
  }
  else {
  }
  (this) -> nScore += (((ucScoreInc) - ('0')));
  return (ucScoreInc) - ('0');
// end of AddScore
}

// **********************************************************************

int Greed::UpdateTable(int nDeltaX,int nDeltaY,int nNumber)
{
// update table i.e fill table with dots
// the number of dots are given by nNumber
// if table position is already filled with a dot then it returns TRUE
// else FALSE
  int nNewXPos;
  int nNewYPos;
  if (nNumber == 0) {
    return (-1);
  }
  else {
  }
  for (; nNumber > 0; nNumber--) {
// if out of range we should be at the same spot
    nNewXPos = ((((this) -> nXPos) + (nDeltaX)));
    nNewYPos = ((((this) -> nYPos) + (nDeltaY)));
    bool rose_sc_bool_2 = false;
    if ((nNewXPos < 0)) {{
        rose_sc_bool_2 = true;
      }
    }
    else {
      if (((nNewXPos) >= ((this) -> nMaxX))) {
        rose_sc_bool_2 = true;
      }
      else {
      }
    }
// is X out of range
    if (rose_sc_bool_2) {
      return (-1);
    }
    else {
    }
    bool rose_sc_bool_3 = false;
    if ((nNewYPos < 0)) {{
        rose_sc_bool_3 = true;
      }
    }
    else {
      if (((nNewYPos) >= ((this) -> nMaxY))) {
        rose_sc_bool_3 = true;
      }
      else {
      }
    }
// is Y out of range
    if (rose_sc_bool_3) {
      return (-1);
    }
    else {
    }
// in range
    (this) -> nXPos = (nNewXPos);
    (this) -> nYPos = (nNewYPos);
// is it an illegal position ?
    if ((((this) -> GetPlayTable(((this) -> nXPos),((this) -> nYPos)))) == ('.')) {
      return (-1);
    }
    else {
    }
    (this) -> SetPlayTable(((this) -> nXPos),((this) -> nYPos),('.'));
    (this) -> UpdateScreen();
  }
  return 0;
// end of UpdateTable
}

// **********************************************************************

void Greed::FillTable()
{
// fills up the table with random numbers; no zeroes allowed
// also sets up initial X and Y position
// srand (iSeed);				sets random seed
  int nPosition = 0;
  int i;
// buffer for sprintf
  char szBuffer[10];
  char bFilled = (0);
  while(!(bFilled)){
    strcpy(szBuffer,("game"));
    i = 0;
    while((szBuffer[i])){
      if (((szBuffer[i])) != ('0')) {
        ((this) -> cPlayTable)[nPosition++] = (szBuffer[i]);
      }
      else {
      }
      i++;
      if ((nPosition) >= (((this) -> nMaxX) * ((this) -> nMaxY))) {
        bFilled = ((-1));
        ((this) -> cPlayTable)[nPosition] = '\0';
        break; 
      }
      else {
      }
    }
  }
  (this) -> nXPos = ((50) % ((this) -> nMaxX));
  (this) -> nYPos = ((50) % ((this) -> nMaxY));
// end of FillTable
}

// **********************************************************************

void Greed::NewGame(int Seed)
{
// used at start of game to fill up the table and update the screen
  (this) -> iSeed = Seed;
  (this) -> FillTable();
  (this) -> nScore = (((((this) -> GetPlayTable(((this) -> nXPos),((this) -> nYPos)))) - ('0')));
  (this) -> SetPlayTable(((this) -> nXPos),((this) -> nYPos),('.'));
  (this) -> FillScreen();
// end of NewGame
}

// **********************************************************************

int Greed::SaveGame()
{
// saves the current game
  strcpy(sBuffer,("game"));
  strcat(sBuffer,((this) -> cPlayTable));
  code(sBuffer,4096);
// write to file
// WriteFile (szSaveFile, sBuffer, SAVE_FILE_SIZE);
  return 1;
// end of SaveGame
}

// **********************************************************************

int Greed::RestoreGame()
{
// restores the game
  if ((strcpy(sBuffer,("game")))) {
    return (-1);
  }
  else {
  }
  code(sBuffer,4096);
  (this) -> nScore = (((this) -> iSeed = (((this) -> nXPos = ((this) -> nYPos = ((this) -> nMaxX = ((this) -> nMaxY = (0))))))));
  char *cTempPlayTable = "game";
  if (cTempPlayTable == ((0))) {
    (this) -> Error(0);
    return (-1);
  }
  else {
  }
// free allocated table and allocate new size
  (this) -> cPlayTable = ((0));
  (this) -> cPlayTable = cTempPlayTable;
  strcpy(((this) -> cPlayTable),("game"));
  (this) -> FillScreen();
  return 0;
}

// **********************************************************************

Greed::Greed(uint nXmax,uint nYmax,char *pszScoreFile,char *pszSaveFile)
{
  if ((nXmax == (0))) {
// checks validity of table size
    (this) -> nMaxX = (DEFAULT_X);
  }
  else {
// checks validity of table size
    (this) -> nMaxX = nXmax;
  }
  if ((nYmax == (0))) {
    (this) -> nMaxY = (DEFAULT_Y);
  }
  else {
    (this) -> nMaxY = nYmax;
  }
  (this) -> cPlayTable = ((0));
  bool rose_sc_bool_4 = false;
  if ((pszScoreFile == ((0)))) {{
      rose_sc_bool_4 = true;
    }
  }
  else {
    if (((( *pszScoreFile)) == ('\0'))) {
      rose_sc_bool_4 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_4) {
    strcpy(((this) -> szScoreFile),("greed.sco"));
  }
  else {
    strcpy(((this) -> szScoreFile),pszScoreFile);
  }
  bool rose_sc_bool_5 = false;
  if ((pszSaveFile == ((0)))) {{
      rose_sc_bool_5 = true;
    }
  }
  else {
    if (((( *pszSaveFile)) == ('\0'))) {
      rose_sc_bool_5 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_5) {
    strcpy(((this) -> szSaveFile),("greed.sav"));
  }
  else {
    strcpy(((this) -> szSaveFile),pszSaveFile);
  }
// end of greed constructor
}

// **********************************************************************

void Greed::Initialize()
{
// Gets memory for the playing table
  (this) -> cPlayTable = ("game");
  if (((this) -> cPlayTable) == ((0))) 
{
    (this) -> Error(0);
// exit (1);
  }
  else {
  }
  ((this) -> cPlayTable)[(((this) -> nMaxX) * ((this) -> nMaxY)) + (1)] = '\0';
  (this) -> NewGame(0);
// initialize the score card
  if ((strcpy(sBuffer,("game")))) {
// unable to read score file
// create new score card
    struct Score dummy;
    (dummy.szName)[0] = '\0';
    dummy.nScore = (0);
    dummy.iSeed = 0;
    for (int i = 0; i < MAX_SCORE; i++) {
      strcpy((((this) -> ScoreCard)[i].szName),(dummy.szName));
      ((this) -> ScoreCard)[i].nScore = (dummy.nScore);
      ((this) -> ScoreCard)[i].iSeed = (dummy.iSeed);
    }
  }
  else {
    code(sBuffer,(((sizeof(Score )) * (MAX_SCORE))));
    strcpy(sBuffer,("game"));
  }
}

// **********************************************************************

Greed::~Greed()
{
  if (((this) -> cPlayTable)) {
    (this) -> cPlayTable = ((0));
  }
  else {
  }
}

// **********************************************************************

int Greed::MoveNorth()
{
  return (this) -> UpdateTable(0,(-1),(this) -> AddScore(0,(-1)));
}


int Greed::MoveSouth()
{
  return (this) -> UpdateTable(0,1,(this) -> AddScore(0,1));
}


int Greed::MoveEast()
{
  return (this) -> UpdateTable(1,0,(this) -> AddScore(1,0));
}


int Greed::MoveWest()
{
  return (this) -> UpdateTable((-1),0,(this) -> AddScore((-1),0));
}


int Greed::MoveNorthEast()
{
  return (this) -> UpdateTable(1,(-1),(this) -> AddScore(1,(-1)));
}


int Greed::MoveSouthEast()
{
  return (this) -> UpdateTable(1,1,(this) -> AddScore(1,1));
}


int Greed::MoveNorthWest()
{
  return (this) -> UpdateTable((-1),(-1),(this) -> AddScore((-1),(-1)));
}


int Greed::MoveSouthWest()
{
  return (this) -> UpdateTable((-1),1,(this) -> AddScore((-1),1));
}

// **********************************************************************

void Dosgreed::UpdateScreen()
{
#if 0
// updates screen and score
// restore cursor back to its position
#endif
// end of UpdateScreen
}

// **********************************************************************

void Dosgreed::FillScreen()
{
#if 0
// fills screen with the numbers and shows initial score
// find current Y position
// display one line
// has cursor gone to the next line
// if not make it go to
// the next line
// restore cursor back to its position
#endif
// end of FillScree
}

// **********************************************************************

void Dosgreed::GameEnd()
{
  int CurrScore;
  char szName[22];
//gotoxy (nXPos + 1, nYPos + 1);
//putch ('*');
//SplSound ();
  MsgAtBot(("Viciously strike a key to continue"));
  CurrScore = (this) -> IsHighScore();
  if (CurrScore < MAX_SCORE) {
//gotoxy (15, nMaxY + 1); clreol ();
//cputs ("Thou art a Greedy Pig; Enter thy name ");
    szName[0] = ((char )20);
// insert name
    (this) -> InsertHighScore(("a"));
//gotoxy (15, nMaxY + 1); clreol ();
    (this) -> ShowScore(CurrScore);
    MsgAtBot(("Viciously strike a key to continue"));
  }
  else {
  }
// end of GameEnd
}

// **********************************************************************

int Dosgreed::ReadFile(char *pszFileName,char *psBuffer,int iSize)
{
  int iHandle;
  int iBytesRead;
//open (pszFileName, O_BINARY | O_RDWR | O_CREAT,
  iHandle = 1;
//S_IREAD | S_IWRITE);	// flags for create file
// error in opening file
  if (iHandle == (-1)) {
    (this) -> Error(0);
    return (-1);
  }
  else {
  }
//read (iHandle, psBuffer, iSize);
  iBytesRead = 1;
// error in reading file
  if (iBytesRead == (-1)) {
    (this) -> Error(0);
    return (-1);
  }
  else {
  }
// invalid file
  if (iBytesRead != iSize) {
    (this) -> Error(("Could Not Read File"));
    return (-1);
  }
  else {
  }
// close (iHandle);
  return 0;
// end of ReadFile
}

// **********************************************************************

int Dosgreed::WriteFile(char *pszFileName,char *psBuffer,int iSize)
{
  int iHandle;
  int iBytesWritten;
//open (pszFileName, O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
  iHandle = 1;
//	S_IREAD | S_IWRITE);		// flags for create file
// error in opening file
  if (iHandle == (-1)) {
    (this) -> Error(0);
    return (-1);
  }
  else {
  }
// write (iHandle, psBuffer, iSize);
  iBytesWritten = 1;
// error in writing file
  if (iBytesWritten == (-1)) {
    (this) -> Error(0);
    return (-1);
  }
  else {
  }
// invalid file
  if (iBytesWritten != iSize) {
    (this) -> Error(("Could Not Write File"));
  }
  else {
  }
// close (iHandle);
  return 0;
// end of ReadFile
}

// **********************************************************************

void Dosgreed::Error(int ErrorNo)
{
  char *ErrMsg;
//strerror (ErrorNo);
  strcpy(ErrMsg,("err"));
// ErrMsg [strlen (ErrMsg) - 1] = '\0';
  MsgAtBot(ErrMsg);
// end of function Error
}

// **********************************************************************

void Dosgreed::Error(char *ErrorMsg)
{
  MsgAtBot(ErrorMsg);
}

// **********************************************************************

void Dosgreed::ShowScore(int HighLight)
{
// Highlight = -1 if no high light wanted
  const int ScoreStart = 7;
//window (10, ScoreStart, 70, ScoreStart + MAX_SCORE + 2);
//textcolor (BLUE); textbackground (CYAN);
//clrscr ();
// show heading
//window (13, ScoreStart, 70, ScoreStart + MAX_SCORE + 2);
//cprintf ("                   The Top Ten Pigs\n\r");
//cprintf ("%-5s%-30s%-10s%-10s", "No", "Name", "Seed", "Score");
//window (13, ScoreStart + 2, 70, ScoreStart + MAX_SCORE + 2);
//textcolor (YELLOW);
// show hi scores
  struct Score *HiScore;
  for (int i = 0; i < MAX_SCORE; i++) 
{
    HiScore = (this) -> GetHighScore(i);
//gotoxy (1, i + 1);
//cprintf ("%-5d%-30s%-10d%-10d", i + 1, HiScore.szName,
//	HiScore.iSeed, HiScore.nScore);
  }
//window (11, ScoreStart + 2, 70, ScoreStart + MAX_SCORE + 2);
  if (HighLight != (-1)) 
{
//gotoxy (1, HighLight + 1);
//putch ('*');
  }
  else {
  }
// restore window
// struct text_info TextInfo;
// gettextinfo (&TextInfo);
// window (1, 1, TextInfo.screenwidth, TextInfo.screenheight);
// textcolor (LIGHTGRAY); textbackground (BLACK);
// end of function ShowScore
}

// **********************************************************************

void Dosgreed::Setup()
{
// textmode (C4350);			// VGA 50 line mode
// textcolor (LIGHTGRAY); textbackground (BLACK);
  nLastLine = (DEFAULT_Y + 1);
  nLastCol = DEFAULT_X;
// randomize ();
  (this) -> Initialize();
}

// **********************************************************************

void Dosgreed::PlayGame()
{
  int bGameEnd = 0;
  char ch1;
  char ch2;
  (this) -> Setup();
  for (; ; ) 
{
    while(!(bGameEnd))
{
// tolower (getch ());
      ch1 = 'a';
      if ((ch1) == 0) {
// getch ();
        ch2 = 'b';
      }
      else {
        ch2 = ch1;
      }
      bGameEnd = (this) -> MoveNorthWest();
      bGameEnd = (this) -> MoveNorth();
      bGameEnd = (this) -> MoveNorthEast();
      bGameEnd = (this) -> MoveWest();
      bGameEnd = (this) -> MoveEast();
      bGameEnd = (this) -> MoveSouthWest();
      bGameEnd = (this) -> MoveSouth();
      bGameEnd = (this) -> MoveSouthEast();
      ShowHelp();
      (this) -> FillScreen();
      if (((this) -> SaveGame())) {
        (this) -> Error(("Game could not be Saved"));
      }
      else {
      }
      if (((this) -> RestoreGame())) {
        (this) -> Error(("Game could not be Restored"));
      }
      else {
      }
      (this) -> ShowScore((-1));
      (this) -> FillScreen();
      (this) -> NewGame(0);
{
        int Number;
        Number = GetInteger();
// valid number
        if (Number != (-1)) {
          (this) -> NewGame(Number);
        }
        else {
          (this) -> FillScreen();
        }
      }
      if (bGameEnd) 
{
        (this) -> GameEnd();
        do 
{
// toupper (getch ());
          ch1 = 'c';
          rose_sc_label_0:
          if (((ch1) != ('N'))) {
            if (((ch1) != ('Y'))) {
            }
            else {
              break; 
            }
          }
          else {{
              break; 
            }
          }
//if (ch1 == 0) getch ();
        }
while (true);
        if ((ch1) == ('N')) {
        }
        else {
          (this) -> NewGame(0);
          bGameEnd = 0;
        }
// end of check if game end
      }
      else {
      }
// end of while loop
    }
// end of infinite loop
  }
// end of function PlayGame
}

int SoundFlag = 0;
int nLastLine;
int nLastCol;
// **********************************************************************

void Sound()
{
  if (SoundFlag) 
{
//sound (340); delay (50);
//sound (240); delay (50);
//nosound ();
  }
  else {
  }
}

// **********************************************************************

void SplSound()
{
  if (SoundFlag) 
{
//sound (500); delay (50);
//sound (400); delay (50);
//sound (300); delay (50);
//sound (200); delay (50);
//nosound ();
  }
  else {
  }
}

// **********************************************************************

void MsgAtBot(char *msg)
{
// displays a message in the middle of the last line of the screen
  char ch;
  int x;
  int y;
  int NewXPos;
// save current cursor posn
  x = 0;
  y = 0;
  NewXPos = ((nLastCol - 0) >> 1);
  if ((NewXPos > 12)) {
// should spill into Score: 12345
    NewXPos = NewXPos;
  }
  else {
// should spill into Score: 12345
    NewXPos = 15;
  }
//gotoxy (NewXPos, nLastLine);
//cputs (msg);
//ch = getch (); if (ch == 0) getch ();	// flush extra keys
//gotoxy (15, nLastLine); clreol ();
//gotoxy (x, y);
}

// **********************************************************************

void ShowHelp()
{
#if 0
// shows the help screen
// restore window
#endif
// end of function ShowHelp
}

// **********************************************************************

int GetInteger()
{
  return 0;
#if 0
// ESC key
#endif
// end of function GetInteger
}

// **********************************************************************

int main()
{
/* can be 49 */
  class Dosgreed PlayTable((DEFAULT_X),(DEFAULT_Y));
  PlayTable.PlayGame();
  return 0;
}

