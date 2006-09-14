// Listings 7.1 and 7.2 from "The C++ Workbook" by Wiener and Pinson.
// Addison-Wesley 1990.

void strcpy(char *nam1,char *nam2)
{
   *nam1 =  *nam2;
}


int strlen(char *nam1)
{
  return 0;
}


class Parent 
{
  protected: char *lastName;
  

  public: inline Parent()
{
// was char[5];
    (this) -> lastName = (new char );
    strcpy(((this) -> lastName),("None"));
  }

  

  inline Parent(char *aLastName)
{
// was char[strlen(aLastName) + 1]
    strlen(aLastName) , (this) -> lastName = (new char );
    strcpy(((this) -> lastName),aLastName);
  }

  

  inline Parent(class Parent &aParent)
{
    strlen((aParent.lastName)) , (this) -> lastName = (new char );
    strcpy(((this) -> lastName),(aParent.lastName));
  }

  

  inline char *getLastName()
{
    return (this) -> lastName;
  }

  

  inline void setLastName(char *aName)
{
    strlen(aName) , (this) -> lastName = (new char );
    strcpy(((this) -> lastName),aName);
  }

  

  virtual inline void answerName()
{
    (this) -> lastName;
  }

  

  inline ~Parent()
{
    delete ((this) -> lastName);
  }

}

;

class Child : public Parent
{
  protected: char *firstName;
  

  public: inline Child()
{
    (this) -> firstName = (new char );
    strcpy(((this) -> firstName),("None"));
  }

  

  inline Child(char *aLastName,char *aFirstName) : Parent(aLastName)
{
    strlen(aFirstName) , (this) -> firstName = (new char );
    strcpy(((this) -> firstName),aFirstName);
  }

  

  inline Child(class Child &aChild)
{
    (this) -> setLastName(aChild.getLastName());
    strlen((aChild.firstName)) , (this) -> firstName = (new char );
    strcpy(((this) -> firstName),(aChild.firstName));
  }

  

  inline char *getFirstName()
{
    return (this) -> firstName;
  }

  

  inline void setFirstName(char *aName)
{
    strlen(aName) , (this) -> firstName = (new char );
    strcpy(((this) -> firstName),aName);
  }

  

  inline ~Child()
{
    delete ((this) -> firstName);
  }

  

  virtual inline void answerName()
{
    (this) -> Parent::answerName();
    (this) -> firstName;
  }

}

;

class GrandChild : public Child
{
  private: char *grandFatherName;
  

  public: inline GrandChild(char *aLastName,char *aFirstName,char *aGrandFatherName) : Child(aLastName,aFirstName)
{
    strlen(aGrandFatherName) , (this) -> grandFatherName = (new char );
    strcpy(((this) -> grandFatherName),aGrandFatherName);
  }

  

  inline ~GrandChild()
{
    delete ((this) -> grandFatherName);
  }

  

  virtual inline void answerName()
{
    (this) -> Child::answerName();
    (this) -> grandFatherName;
  }

}

;

int main()
{
  class Parent p(("Jones"));
  class Child c(("Jones"),("Henry"));
  class GrandChild g(("Jones"),("Cynthia"),("Murray"));
  class Parent &f0 = p;
  class Parent &f1 = g;
  class Parent &f2 = c;
  f0.answerName();
  f1.answerName();
  f2.answerName();
}

