#include <iostream>

class Creature
{
private:
    std::string m_name;
    char m_symbol;
    int m_health;
    int m_damagePerAttack;
    int m_gold;

public:
    Creature(const std::string& name, char symbol, int health, int damagePerAttack, int gold)
        : m_name{name}, m_symbol{symbol}, m_health{health}, m_damagePerAttack{damagePerAttack}, m_gold{gold}
    {

    }

    const std::string& getName(void) const {return m_name;}
    char getSymbol(void) const {return m_symbol;}
    int getHealth(void) const {return m_health;}
    int getDamage(void) const {return m_damagePerAttack;}
    int getGold(void) const {return m_gold;}

    void reduceHealth(int value)
    {
        m_health -= value;
    }

    bool isDead(void) {return (m_health <= 0);}

    void addGold(int value) {m_gold += value;}
};

class Player : public Creature
{
private:
    int m_level{};

public:

    Player(const std::string& name, int health = 10, int damagePerAttack = 1, int gold = 0, int level = 1)
        : Creature(name, '@', health, damagePerAttack, gold), m_level{level}
    {

    }

    int getLevel() const { return m_level;}

    bool hasWon(void) {return (m_level >= 30);}
};

class Monster : public Creature
{
public:
	enum class Type
	{
		dragon,
		orc,
		slime,
		max_types
	};

};


int main()
{
	std::cout << "Enter your name: ";
	std::string playerName;
	std::cin >> playerName;

	Player p{ playerName };
	std::cout << "Welcome, " << p.getName() << ".\n";

	std::cout << "You have " << p.getHealth() << " health and are carrying " << p.getGold() << " gold.\n";

	return 0;
}
