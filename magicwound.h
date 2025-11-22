#ifndef CARD_GAME_H
#define CARD_GAME_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <random>
#include <memory>
#include <sstream>
#include <iomanip>

// Better Enums 头文件
#include <enum.h>

// 简易的CRC32校验 (header-only)
namespace crc32 {
    uint32_t calculate(const std::string& input);
    std::string generate_checksum(const std::string& input);
}

// Base64编解码
namespace base64 {
    std::string encode(const std::string &input);
    std::string decode(const std::string &input);
}

// 定义卡牌类型
BETTER_ENUM(CardType, int,
    Creature = 1,    // 生物
    Spell = 2        // 法术  
)

// 定义卡牌元素
BETTER_ENUM(Element, int,
    Physical = 1,    // 物理
    Light = 2,       // 光
    Dark = 3,        // 暗
    Water = 4,       // 水
    Fire = 5,        // 火
    Earth = 6,       // 土
    Wind = 7         // 风
)

// 定义稀有度
BETTER_ENUM(Rarity, int,
    Common = 1,
    Uncommon = 2,
    Rare = 3,
    Mythic = 4, 
    Funny = 5
)

// 定义套牌类型
BETTER_ENUM(DeckType, int,
    Standard = 1,    // 标准套牌
    Casual = 2       // 娱乐套牌
)

// 人物类
class Character {
private:
    std::string id;
    std::string name;
    std::vector<Element> elements;
    int health;
    int energy;
    std::string ability;
    std::string description;

public:
    Character(const std::string& id, const std::string& name, 
              const std::vector<Element>& elements, int health, int energy,
              const std::string& ability, const std::string& description);

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::vector<Element> getElements() const { return elements; }
    int getHealth() const { return health; }
    int getEnergy() const { return energy; }
    std::string getAbility() const { return ability; }
    std::string getDescription() const { return description; }

    bool hasElement(Element element) const;
    void display() const;
    std::string elementToString(Element element) const;
};

// 卡牌类
class Card {
private:
    std::string id;
    std::string name;
    CardType type;
    std::vector<Element> elements;
    int cost;
    Rarity rarity;
    std::string description;
    int attack;
    int defense;
    int health;


public:
    std::string elementToString(Element element) const;
    std::string rarityToString(Rarity rarity) const;
    // 修改构造函数以正确初始化type
    Card(const std::string& id, const std::string& name, const std::vector<Element>& elements, 
         int cost, Rarity rarity, const std::string& description,
         int attack = 0, int defense = 0, int health = 0);

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    CardType getType() const { return type; }
    std::vector<Element> getElements() const { return elements; }
    int getCost() const { return cost; }
    Rarity getRarity() const { return rarity; }
    std::string getDescription() const { return description; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }
    int getHealth() const { return health; }

    bool hasElement(Element element) const;
    std::string serialize() const;
    void display() const;
};

// 套牌类
class Deck {
private:
    std::string name;
    DeckType deckType;
    std::vector<std::shared_ptr<Card>> cards;
    std::vector<std::shared_ptr<Character>> characters;
    std::vector<Element> deckElements;
    std::string deckCode;
    int maxCardLimit;  // 套牌最大卡牌数量限制

    void updateDeckElements();
    void updateDeckCode();
    std::string elementToString(Element element) const;
    std::string cardTypeToString(CardType type) const;
    std::string deckTypeToString() const;

public:
    Deck(const std::string& name, DeckType type = +DeckType::Standard);
    
    void addCard(const std::shared_ptr<Card>& card);
    bool removeCard(const std::string& cardName);
    void addCharacter(const std::shared_ptr<Character>& character);
    bool removeCharacter(const std::string& characterName);
    
    size_t getCardCount() const;
    size_t getCharacterCount() const;
    std::string getName() const;
    DeckType getDeckType() const;
    std::string getDeckCode() const;
    int getMaxCardLimit() const { return maxCardLimit; }
    void setMaxCardLimit(int limit) { maxCardLimit = limit; }
    
    std::map<Element, int> getElementDistribution() const;
    void display() const;
    void shuffle(); // 修改shuffle方法
    
    bool importFromDeckCode(const std::string& code, 
                           const std::vector<std::shared_ptr<Card>>& allCards,
                           const std::vector<std::shared_ptr<Character>>& allCharacters);
    static bool isValidDeckCode(const std::string& code);
    
    bool isValid() const;
};

// 人物数据库类
class CharacterDatabase {
private:
    std::vector<std::shared_ptr<Character>> allCharacters;
    void initializeCharacters();

public:
    CharacterDatabase();
    const std::vector<std::shared_ptr<Character>>& getAllCharacters() const;
    std::shared_ptr<Character> findCharacter(const std::string& name) const;
    std::shared_ptr<Character> findCharacterById(const std::string& id) const;
    std::vector<std::shared_ptr<Character>> getCharactersByElement(Element element) const;
};

// 卡牌数据库类
class CardDatabase {
private:
    std::vector<std::shared_ptr<Card>> allCards;
    void initializeCards();

public:
    CardDatabase();
    const std::vector<std::shared_ptr<Card>>& getAllCards() const;
    std::shared_ptr<Card> findCard(const std::string& name) const;
    std::shared_ptr<Card> findCardById(const std::string& id) const;
    std::vector<std::shared_ptr<Card>> getCardsByType(CardType type) const;
    std::vector<std::shared_ptr<Card>> getCardsByElement(Element element) const;
    std::vector<std::shared_ptr<Card>> getCardsByRarity(Rarity rarity) const;
};

// 游戏管理类
class GameManager {
private:
    CardDatabase cardDB;
    CharacterDatabase characterDB;
    std::vector<Deck> decks;

public:
    void displayAllCards() const;
    void displayAllCharacters() const;
    void createDeck();
    void displayDecks() const;
    void displayDeckDetails() const;
    void exportDeckCode() const;
    void importDeckFromCode();
    void showMenu() const;
    void run();
};

#endif // CARD_GAME_H
