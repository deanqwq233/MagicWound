#include "magicwound.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

using namespace std;

// CRC32 实现 - 使用 Boost
namespace crc32 {
    uint32_t calculate(const std::string& input) {
        boost::crc_32_type result;
        result.process_bytes(input.data(), input.length());
        return result.checksum();
    }

    std::string generate_checksum(const std::string& input) {
        uint32_t crc = calculate(input);
        stringstream ss;
        ss << hex << setw(8) << setfill('0') << crc;
        return ss.str().substr(0, 4);
    }
}

// Base64 实现 - 使用 Boost
namespace base64 {
    using namespace boost::archive::iterators;
    
    // Base64 编码器
    typedef base64_from_binary<transform_width<string::const_iterator, 6, 8>> base64_enc;
    // Base64 解码器  
    typedef transform_width<binary_from_base64<string::const_iterator>, 8, 6> base64_dec;

    string encode(const string &input) {
        if (input.empty()) return string();
        
        string encoded(base64_enc(input.begin()), base64_enc(input.end()));
        
        // 添加填充
        size_t padding = (3 - input.length() % 3) % 3;
        for (size_t i = 0; i < padding; i++) {
            encoded.push_back('=');
        }
        
        return encoded;
    }

    string decode(const string &input) {
        if (input.empty()) return string();
        
        // 移除填充
        size_t padding = count(input.begin(), input.end(), '=');
        string clean_input = input.substr(0, input.length() - padding);
        
        if (clean_input.empty()) return string();
        
        try {
            string decoded(base64_dec(clean_input.begin()), base64_dec(clean_input.end()));
            
            // 移除由于填充产生的空字符
            size_t real_length = (clean_input.length() * 6) / 8;
            if (real_length < decoded.length()) {
                decoded.resize(real_length);
            }
            
            return decoded;
        } catch (...) {
            return string();
        }
    }
}

// Character 实现
Character::Character(const string& id, const string& name, 
                   const vector<Element>& elements, int health, int energy,
                   const string& ability, const string& description,
                   const string& passive_ability, const string& passive_description)
    : id(id), name(name), elements(elements), health(health), 
      energy(energy), ability(ability), description(description), 
      passive_ability(passive_ability), passive_description(passive_description) {}

bool Character::hasElement(Element element) const {
    return find(elements.begin(), elements.end(), element) != elements.end();
}

void Character::display() const {
    cout << "角色: " << name << endl;
    cout << "元素: ";
    for (const auto& element : elements) {
        cout << elementToString(element) << " ";
    }
    cout << endl;
    cout << "生命值: " << health << endl;
    cout << "能量: " << energy << endl;
    cout << "能力: " << ability << endl;
    cout << "描述: " << description << endl;
    cout << "被动能力: " << passive_ability << endl;
    cout << "被动描述: " << passive_description << endl;
    cout << "ID: " << id << endl;
    cout << "------------------------" << endl;
}

string Character::elementToString(Element element) const {
    switch(element) {
        case +Element::Physical: return "物理";
        case +Element::Light: return "光";
        case +Element::Dark: return "暗";
        case +Element::Water: return "水";
        case +Element::Fire: return "火";
        case +Element::Earth: return "土";
        case +Element::Wind: return "风";
        default: return "未知";
    }
}

// Card 实现 - 修改构造函数
Card::Card(const string& id, const string& name, const vector<Element>& elements, 
     int cost, Rarity rarity, const string& description,
     int attack, int defense, int health)
    : id(id), name(name), 
      type((attack == 0 && defense == 0 && health == 0) ? +CardType::Spell : +CardType::Creature), // 正确初始化type
      elements(elements), cost(cost), rarity(rarity), description(description), 
      attack(attack), defense(defense), health(health) {}

bool Card::hasElement(Element element) const {
    return find(elements.begin(), elements.end(), element) != elements.end();
}

string Card::serialize() const {
    return id;
}

void Card::display() const {
    cout << "名称: " << name << endl;
    cout << "类型: " << (type == +CardType::Creature ? "生物" : "法术") << endl;
    
    cout << "元素: ";
    for (const auto& element : elements) {
        cout << elementToString(element) << " ";
    }
    cout << endl;
    
    cout << "费用: " << cost << endl;
    cout << "稀有度: " << rarityToString(rarity) << endl;
    cout << "描述: " << description << endl;
    
    if (type == +CardType::Creature) {
        cout << "攻击/防御/生命: " << attack << "/" << defense << "/" << health << endl;
    }
    cout << "ID: " << id << endl;
    cout << "------------------------" << endl;
}

string Card::elementToString(Element element) const {
    switch(element) {
        case +Element::Physical: return "物理";
        case +Element::Light: return "光";
        case +Element::Dark: return "暗";
        case +Element::Water: return "水";
        case +Element::Fire: return "火";
        case +Element::Earth: return "土";
        case +Element::Wind: return "风";
        default: return "未知";
    }
}

string Card::rarityToString(Rarity rarity) const {
    switch(rarity) {
        case +Rarity::Common: return "普通";
        case +Rarity::Uncommon: return "罕见";
        case +Rarity::Rare: return "稀有";
        case +Rarity::Mythic: return "神话";
        case +Rarity::Funny: return "趣味";
        default: return "未知";
    }
}

// Deck 实现
Deck::Deck(const string& name, DeckType type) : name(name), deckType(type), maxCardLimit(20) {
    updateDeckCode();
}

void Deck::addCard(const shared_ptr<Card>& card) {
    // 检查标准牌组限制
    if (deckType == +DeckType::Standard && card->getRarity() == +Rarity::Funny) {
        cout << "标准牌组不能携带趣味稀有度的卡牌: " << card->getName() << endl;
        return;
    }
    
    // 检查最大卡牌数量
    if (cards.size() >= maxCardLimit) {
        cout << "牌组已达到最大卡牌数量 (" << maxCardLimit << "张)" << endl;
        return;
    }
    
    cards.push_back(card);
    updateDeckElements();
    updateDeckCode();
}

bool Deck::removeCard(const string& cardName) {
    auto it = find_if(cards.begin(), cards.end(),
        [&cardName](const shared_ptr<Card>& card) {
            return card->getName() == cardName;
        });
    
    if (it != cards.end()) {
        cards.erase(it);
        updateDeckElements();
        updateDeckCode();
        return true;
    }
    return false;
}

void Deck::addCharacter(const shared_ptr<Character>& character) {
    if (characters.size() >= 3) {
        cout << "牌组最多只能有3个角色" << endl;
        return;
    }
    
    characters.push_back(character);
    updateDeckCode();
}

bool Deck::removeCharacter(const string& characterName) {
    auto it = find_if(characters.begin(), characters.end(),
        [&characterName](const shared_ptr<Character>& character) {
            return character->getName() == characterName;
        });
    
    if (it != characters.end()) {
        characters.erase(it);
        updateDeckCode();
        return true;
    }
    return false;
}

size_t Deck::getCardCount() const {
    return cards.size();
}

size_t Deck::getCharacterCount() const {
    return characters.size();
}

string Deck::getName() const {
    return name;
}

DeckType Deck::getDeckType() const {
    return deckType;
}

string Deck::getDeckCode() const {
    return deckCode;
}

map<Element, int> Deck::getElementDistribution() const {
    map<Element, int> distribution;
    for (const auto& element : deckElements) {
        distribution[element] = 0;
    }
    
    for (const auto& card : cards) {
        for (const auto& element : card->getElements()) {
            distribution[element]++;
        }
    }
    return distribution;
}

void Deck::display() const {
    cout << "\n=== 牌组详情 ===" << endl;
    cout << "牌组名称: " << name << endl;
    cout << "牌组类型: " << deckTypeToString() << endl;
    cout << "卡牌数量: " << cards.size() << "/" << maxCardLimit << endl;
    cout << "角色数量: " << characters.size() << "/3" << endl;
    cout << "牌组代码: " << deckCode << endl;
    
    auto distribution = getElementDistribution();
    cout << "元素分布:" << endl;
    for (const auto& pair : distribution) {
        cout << "  " << elementToString(pair.first) << ": " << pair.second << " 张" << endl;
    }
    
    map<CardType, int> typeCount;
    for (const auto& card : cards) {
        typeCount[card->getType()]++;
    }
    
    cout << "类型分布:" << endl;
    for (const auto& pair : typeCount) {
        cout << "  " << cardTypeToString(pair.first) << ": " << pair.second << " 张" << endl;
    }
    
    cout << "角色列表:" << endl;
    for (const auto& character : characters) {
        cout << "- " << character->getName() << " (生命:" << character->getHealth();
        cout << ", 能量:" << character->getEnergy() << ")" << endl;
    }
    
    cout << "卡牌列表:" << endl;
    for (const auto& card : cards) {
        cout << "- " << card->getName() << " (费用:" << card->getCost();
        cout << ", 元素:";
        for (const auto& element : card->getElements()) {
            cout << elementToString(element) << " ";
        }
        cout << ")" << endl;
    }
}

// 修改shuffle方法 - 使用 Boost 随机数
void Deck::shuffle() {
    static boost::random::mt19937 rng(static_cast<unsigned int>(time(0)));
    boost::random::uniform_int_distribution<> dist;
    
    for (size_t i = cards.size() - 1; i > 0; --i) {
        size_t j = dist(rng) % (i + 1);
        swap(cards[i], cards[j]);
    }
}

bool Deck::importFromDeckCode(const string& code, 
                             const vector<shared_ptr<Card>>& allCards,
                             const vector<shared_ptr<Character>>& allCharacters) {
    if (!isValidDeckCode(code)) {
        return false;
    }
    
    try {
        string decoded = base64::decode(code);
        size_t separator = decoded.find('|');
        if (separator == string::npos) {
            return false;
        }
        
        string dataPart = decoded.substr(0, separator);
        string checksum = decoded.substr(separator + 1);
        
        if (crc32::generate_checksum(dataPart) != checksum) {
            return false;
        }
        
        // 解析牌组数据
        istringstream iss(dataPart);
        string deckName, deckTypeStr;
        getline(iss, deckName, ';');
        getline(iss, deckTypeStr, ';');
        
        this->name = deckName;
        
        // 修改枚举初始化方式
        int deckTypeValue = stoi(deckTypeStr);
        if (deckTypeValue == +DeckType::Standard) {
            this->deckType = +DeckType::Standard;
        } else {
            this->deckType = +DeckType::Casual;
        }
        
        cards.clear();
        characters.clear();
        
        // 读取角色ID
        string characterIds;
        getline(iss, characterIds, ';');
        vector<string> charIdList;
        boost::split(charIdList, characterIds, boost::is_any_of(","));
        
        for (const auto& charId : charIdList) {
            if (!charId.empty()) {
                auto character = find_if(allCharacters.begin(), allCharacters.end(),
                    [&charId](const shared_ptr<Character>& c) {
                        return c->getId() == charId;
                    });
                
                if (character != allCharacters.end()) {
                    characters.push_back(*character);
                }
            }
        }
        
        // 读取卡牌ID
        string cardIds;
        getline(iss, cardIds, ';');
        vector<string> cardIdList;
        boost::split(cardIdList, cardIds, boost::is_any_of(","));
        
        for (const auto& cardId : cardIdList) {
            if (!cardId.empty()) {
                auto card = find_if(allCards.begin(), allCards.end(),
                    [&cardId](const shared_ptr<Card>& c) {
                        return c->getId() == cardId;
                    });
                
                if (card != allCards.end()) {
                    cards.push_back(*card);
                }
            }
        }
        
        // 读取卡牌限制
        string limitStr;
        if (getline(iss, limitStr, ';')) {
            maxCardLimit = stoi(limitStr);
        }
        
        updateDeckElements();
        deckCode = code;
        return true;
    } catch (...) {
        return false;
    }
}

bool Deck::isValidDeckCode(const string& code) {
    try {
        string decoded = base64::decode(code);
        size_t separator = decoded.find('|');
        if (separator == string::npos) {
            return false;
        }
        
        string dataPart = decoded.substr(0, separator);
        string checksum = decoded.substr(separator + 1);
        
        return crc32::generate_checksum(dataPart) == checksum;
    } catch (...) {
        return false;
    }
}

bool Deck::isValid() const {
    return cards.size() >= 20 && characters.size() == 3;
}

void Deck::updateDeckElements() {
    deckElements.clear();
    vector<Element> allElements;
    
    for (const auto& card : cards) {
        for (const auto& element : card->getElements()) {
            allElements.push_back(element);
        }
    }
    
    sort(allElements.begin(), allElements.end());
    auto last = unique(allElements.begin(), allElements.end());
    allElements.erase(last, allElements.end());
    
    deckElements = allElements;
}

void Deck::updateDeckCode() {
    // 构建牌组数据字符串
    stringstream data;
    data << name << ";";
    data << +deckType << ";";
    
    // 角色ID
    for (size_t i = 0; i < characters.size(); ++i) {
        data << characters[i]->getId();
        if (i < characters.size() - 1) {
            data << ",";
        }
    }
    data << ";";
    
    // 卡牌ID
    for (size_t i = 0; i < cards.size(); ++i) {
        data << cards[i]->serialize();
        if (i < cards.size() - 1) {
            data << ",";
        }
    }
    data << ";";
    
    // 卡牌限制
    data << maxCardLimit << ";";
    
    string dataStr = data.str();
    string checksum = crc32::generate_checksum(dataStr);
    
    // 进行Base64编码
    string combined = dataStr + "|" + checksum;
    deckCode = base64::encode(combined);
}

string Deck::elementToString(Element element) const {
    switch(element) {
        case +Element::Physical: return "物理";
        case +Element::Light: return "光";
        case +Element::Dark: return "暗";
        case +Element::Water: return "水";
        case +Element::Fire: return "火";
        case +Element::Earth: return "土";
        case +Element::Wind: return "风";
        default: return "未知";
    }
}

string Deck::cardTypeToString(CardType type) const {
    switch(type) {
        case +CardType::Creature: return "生物";
        case +CardType::Spell: return "法术";
        default: return "未知";
    }
}

string Deck::deckTypeToString() const {
    switch(deckType) {
        case +DeckType::Standard: return "标准牌组";
        case +DeckType::Casual: return "休闲牌组";
        default: return "未知";
    }
}

// CharacterDatabase 实现
CharacterDatabase::CharacterDatabase() {
    initializeCharacters();
}

void CharacterDatabase::initializeCharacters() {
    // 初始化人物
    allCharacters.push_back(make_shared<Character>(
        "xxmlt", "金天", 
        vector<Element>{+Element::Water}, 25, 15,
        "治疗", "消耗5点魔力，指定一个友方目标获得5点生命值。",
        "死生", "\033[1m每局对战限一次\033[0m，当我方人物受到致命伤时，不使其下场,而是使生命值降为1。"
    ));
    allCharacters.push_back(make_shared<Character>(
        "neko", "三金", 
        vector<Element>{+Element::Wind}, 20, 25,
        "吹飞", "消耗10点魔力，选择一项：指定一个对方目标下场；或令一个效果消失。",
        "",""
    ));
    allCharacters.push_back(make_shared<Character>(
        "soybeanmilk", "江源", 
        vector<Element>{+Element::Light}, 20, 20,
        "恢复", "消耗10点魔力将场上存在的其他人或魔物状态恢复至上回合结束时。（第二回合解锁）",
        "无","\033[3m什么？都能回溯了你还想要被动？\033[0m"
    ));
}

const vector<shared_ptr<Character>>& CharacterDatabase::getAllCharacters() const {
    return allCharacters;
}

shared_ptr<Character> CharacterDatabase::findCharacter(const string& name) const {
    auto it = find_if(allCharacters.begin(), allCharacters.end(),
        [&name](const shared_ptr<Character>& character) {
            return character->getName() == name;
        });
    
    return (it != allCharacters.end()) ? *it : nullptr;
}

shared_ptr<Character> CharacterDatabase::findCharacterById(const string& id) const {
    auto it = find_if(allCharacters.begin(), allCharacters.end(),
        [&id](const shared_ptr<Character>& character) {
            return character->getId() == id;
        });
    
    return (it != allCharacters.end()) ? *it : nullptr;
}

vector<shared_ptr<Character>> CharacterDatabase::getCharactersByElement(Element element) const {
    vector<shared_ptr<Character>> result;
    for (const auto& character : allCharacters) {
        if (character->hasElement(element)) {
            result.push_back(character);
        }
    }
    return result;
}

// CardDatabase 实现
CardDatabase::CardDatabase() {
    initializeCards();
}

void CardDatabase::initializeCards() {
    // 初始化带有各种元素的卡牌

    allCards.push_back(make_shared<Card>(
        "madposion", "狂乱药水", 
        vector<Element>{+Element::Water}, 15, Rarity::Mythic,
        "本回合中，目标人物卡牌释放三次，在其魔力不足时以三倍于魔力值消耗的生命替代。"
    ));
	allCards.push_back(make_shared<Card>(
        "organichemistry", "魔药学领城大神！", 
        vector<Element>{+Element::Water}, 9, Rarity::Mythic,
        "本局对战中，你的药水魔力消耗减少（2）。随机获取3张药水。"
    ));
	allCards.push_back(make_shared<Card>(
        "slowdown", "缓慢药水", 
        vector<Element>{+Element::Water}, 5, Rarity::Rare,
        "直到你的下个回合，你对手的牌魔力消耗增加（2）。"
    ));
	allCards.push_back(make_shared<Card>(
        "Timeelder", "时空限速", 
        vector<Element>{+Element::Dark}, 5, Rarity::Rare,
        "直到你的下个回合，你对手不能使用5张以上的牌。（已使用%d张）"
    ));
	allCards.push_back(make_shared<Card>(
        "LGBTQ", "多彩药水", 
        vector<Element>{+Element::Water}, 3, Rarity::Rare,
        "本回合中，你的牌是所有属性。"
    ));
	allCards.push_back(make_shared<Card>(
        "Lazarus,Arise!", "起尸", 
        vector<Element>{+Element::Dark}, 2, Rarity::Rare,
        "复活一个人物，并具有25%的生命（向下取整），在你的的结束时，将其消灭。如果其已死亡，致为使其无法复活。"
    ));
	allCards.push_back(make_shared<Card>(
        "DontForgotMe", "瓶装记忆", 
        vector<Element>{+Element::Water}, 5, Rarity::Rare,
        "这张牌是药水。将目标玩家卡组中的8张牌洗入你的牌库，其魔力消耗减少（2）。"
    ));
	allCards.push_back(make_shared<Card>(
        "TheCardLetMeWin", "记忆屏蔽", 
        vector<Element>{+Element::Water}, 6, Rarity::Rare,
        "摧毁你对手牌库顶和底各2张牌。"
    ));
	allCards.push_back(make_shared<Card>(
        "TheCardLetYouLose", "记忆摧毁", 
        vector<Element>{+Element::Water}, 2, Rarity::Rare,
        "摧毁\033[3m你\033[0m和对手牌库顶和底各2张牌。然后如果你的牌库为空，你输掉游戏。"
    ));
	allCards.push_back(make_shared<Card>(
        "whAt", "你说啥？", 
        vector<Element>{+Element::Water}, 2, Rarity::Rare,
        "摧毁对手牌库中的1张牌。然后摧毁所有同名卡（无论其在哪里）。"
    ));
	allCards.push_back(make_shared<Card>(
        "balance", "平衡", 
        vector<Element>{+Element::Light, +Element::Dark}, 4, Rarity::Rare,
        "弃掉你的手牌。抽等量的牌。"
    ));
	allCards.push_back(make_shared<Card>(
        "TearAll", "遗忘灵药", 
        vector<Element>{+Element::Water, +Element::Dark}, 18, Rarity::Rare,
        "摧毁你对手的牌库。将你对手弃牌堆中的10张牌洗入其牌库，它们的魔力消耗增加（2）。"
    ));
	allCards.push_back(make_shared<Card>(
        "Wordle", "Wordle", 
        vector<Element>{+Element::Physical}, 4, Rarity::Funny,
        "使你对手下回合造成的伤害额外乘上今日Wordle的通关率。"
    ));
	allCards.push_back(make_shared<Card>(
        "IDontcar", "窝不载乎", 
        vector<Element>{+Element::Physical}, 2, Rarity::Funny,
        "你的对手发送的表情改为汽车鸣笛声。\033[3m呜呜呜！\033[0m"
    ));
}

const vector<shared_ptr<Card>>& CardDatabase::getAllCards() const {
    return allCards;
}

shared_ptr<Card> CardDatabase::findCard(const string& name) const {
    auto it = find_if(allCards.begin(), allCards.end(),
        [&name](const shared_ptr<Card>& card) {
            return card->getName() == name;
        });
    
    return (it != allCards.end()) ? *it : nullptr;
}

shared_ptr<Card> CardDatabase::findCardById(const string& id) const {
    auto it = find_if(allCards.begin(), allCards.end(),
        [&id](const shared_ptr<Card>& card) {
            return card->getId() == id;
        });
    
    return (it != allCards.end()) ? *it : nullptr;
}

vector<shared_ptr<Card>> CardDatabase::getCardsByType(CardType type) const {
    vector<shared_ptr<Card>> result;
    for (const auto& card : allCards) {
        if (card->getType() == type) {
            result.push_back(card);
        }
    }
    return result;
}

vector<shared_ptr<Card>> CardDatabase::getCardsByElement(Element element) const {
    vector<shared_ptr<Card>> result;
    for (const auto& card : allCards) {
        if (card->hasElement(element)) {
            result.push_back(card);
        }
    }
    return result;
}

vector<shared_ptr<Card>> CardDatabase::getCardsByRarity(Rarity rarity) const {
    vector<shared_ptr<Card>> result;
    for (const auto& card : allCards) {
        if (card->getRarity() == rarity) {
            result.push_back(card);
        }
    }
    return result;
}

// GameManager 实现
void GameManager::displayAllCards() const {
    cout << "=== 所有卡牌 ===" << endl;
    for (const auto& card : cardDB.getAllCards()) {
        card->display();
    }
}

void GameManager::displayAllCharacters() const {
    cout << "=== 所有角色 ===" << endl;
    for (const auto& character : characterDB.getAllCharacters()) {
        character->display();
    }
}

void GameManager::createDeck() {
    string deckName;
    cout << "请输入牌组名称: ";
    cin.ignore();
    getline(cin, deckName);
    
    // 选择牌组类型
    cout << "选择牌组类型:" << endl;
    cout << "1. 标准牌组 (不能携带趣味稀有度卡牌)" << endl;
    cout << "2. 休闲牌组 (可以携带所有卡牌)" << endl;
    cout << "选择: ";
    
    int typeChoice;
    cin >> typeChoice;
    cin.ignore();  // 清除换行符
    
    DeckType deckType = (typeChoice == 1) ? +DeckType::Standard : +DeckType::Casual;
    Deck newDeck(deckName, deckType);
    
    // 选择角色 - 改为按编号选择
	cout << "\n选择3个角色 (输入编号):" << endl;
	auto allChars = characterDB.getAllCharacters();
	for (size_t i = 0; i < allChars.size(); ++i) {
		cout << "[" << i << "] " << allChars[i]->getName() << " (" << allChars[i]->getHealth() << " HP, " << allChars[i]->getEnergy() << " MP)" << endl;
	}
	for (int i = 0; i < 3; i++) {
		cout << "选择第 " << (i + 1) << " 个角色编号: ";
		string idxs;
		getline(cin, idxs);
		int idx = -1;
		try { idx = stoi(idxs); } catch(...) { idx = -1; }
		if (idx < 0 || idx >= (int)allChars.size()) {
			cout << "无效编号，重新选择。" << endl;
			--i;
			continue;
		}
		newDeck.addCharacter(allChars[idx]);
		cout << "已添加角色: " << allChars[idx]->getName() << endl;
	}

	// 选择卡牌 - 改为按编号选择，输入 done 结束
	cout << "\n选择要添加到牌组的卡牌 (输入卡牌编号，输入'done'结束):" << endl;
	// 根据牌组类型过滤卡牌
	vector<shared_ptr<Card>> availableCards;
	if (deckType == +DeckType::Standard) {
		// 标准牌组不能包含Funny稀有度卡牌
		auto allCards = cardDB.getAllCards();
		copy_if(allCards.begin(), allCards.end(), back_inserter(availableCards),
			[](const shared_ptr<Card>& card) {
				return card->getRarity() != +Rarity::Funny;
			});
	} else {
		availableCards = cardDB.getAllCards();
	}
	for (size_t i = 0; i < availableCards.size(); ++i) {
		auto card = availableCards[i];
		cout << "[" << i << "] " << card->getName() << " (" << (card->getType() == +CardType::Creature ? "生物" : "法术")
			<< ", " << card->getCost() << ", " << card->rarityToString(card->getRarity()) << ")" << endl;
	}
	while (true) {
		cout << "输入卡牌编号或 done: ";
		string line; getline(cin, line);
		if (line == "done") break;
		int cidx = -1;
		try { cidx = stoi(line); } catch(...) { cidx = -1; }
		if (cidx < 0 || cidx >= (int)availableCards.size()) {
			cout << "无效编号。" << endl;
			continue;
		}
		auto card = availableCards[cidx];
		newDeck.addCard(card);
		cout << "已添加卡牌: " << card->getName() << " (" << newDeck.getCardCount() << "/" << newDeck.getMaxCardLimit() << ")" << endl;
	}
    
    // 检查牌组是否有效
    if (newDeck.isValid()) {
        decks.push_back(newDeck);
        cout << "牌组创建成功!" << endl;
    } else {
        cout << "牌组无效! 需要至少20张卡牌和3个角色。" << endl;
        cout << "当前: " << newDeck.getCardCount() << "张卡牌, " << newDeck.getCharacterCount() << "个角色" << endl;
    }
}

void GameManager::displayDecks() const {
    cout << "=== 我的牌组 ===" << endl;
    for (size_t i = 0; i < decks.size(); ++i) {
        string validStatus = decks[i].isValid() ? "有效" : "无效";
        cout << i + 1 << ". " << decks[i].getName() 
                  << " (" << decks[i].getCardCount() << " 张卡牌, " 
                  << decks[i].getCharacterCount() << " 个角色) - " << validStatus << endl;
    }
}

void GameManager::displayDeckDetails() const {
    if (decks.empty()) {
        cout << "没有牌组可以显示。" << endl;
        return;
    }
    
    displayDecks();
    cout << "选择牌组编号: ";
    int choice;
    cin >> choice;
    
    if (choice > 0 && choice <= static_cast<int>(decks.size())) {
        decks[choice - 1].display();
    } else {
        cout << "无效选择" << endl;
    }
}

void GameManager::exportDeckCode() const {
    if (decks.empty()) {
        cout << "没有牌组可以导出" << endl;
        return;
    }
    
    displayDecks();
    cout << "选择要导出的牌组编号: ";
    int choice;
    cin >> choice;
    
    if (choice > 0 && choice <= static_cast<int>(decks.size())) {
        cout << "牌组代码: " << decks[choice - 1].getDeckCode() << endl;
        cout << "请保存此代码以备后续导入。" << endl;
    } else {
        cout << "无效选择" << endl;
    }
}

void GameManager::importDeckFromCode() {
    string deckCode;
    cout << "请输入牌组代码: ";
    cin.ignore();
    getline(cin, deckCode);

    // 先用已有方法尝试导入
    if (!Deck::isValidDeckCode(deckCode)) {
        cout << "无效的牌组代码!" << endl;
        return;
    }

    Deck probe("导入的牌组");
    if (probe.importFromDeckCode(deckCode, cardDB.getAllCards(), characterDB.getAllCharacters())) {
        string newName;
        cout << "牌组导入成功! 请输入新的牌组名称: ";
        getline(cin, newName);
        Deck importedDeck(newName, probe.getDeckType());
        if (importedDeck.importFromDeckCode(deckCode, cardDB.getAllCards(), characterDB.getAllCharacters())) {
            decks.push_back(importedDeck);
            cout << "牌组导入完成!" << endl;
            importedDeck.display();
            return;
        }
        // 若上面失败则继续尝试手动解析（作为回退）
        cout << "警告：自动导入步骤异常，尝试手动解析..." << endl;
    } else {
        cout << "自动导入失败，尝试手动解析..." << endl;
    }

    // 回退：手动解码并解析牌组数据
    try {
        string decoded = base64::decode(deckCode);
        size_t sep = decoded.find('|');
        if (sep == string::npos) {
            cout << "解析失败：找不到校验分隔符" << endl;
            return;
        }

        string dataPart = decoded.substr(0, sep);
        string checksum = decoded.substr(sep + 1);
        if (crc32::generate_checksum(dataPart) != checksum) {
            cout << "解析失败：校验和不匹配" << endl;
            return;
        }

        // data layout: name;type;charIds;cardIds;maxLimit;
        vector<string> parts;
        boost::split(parts, dataPart, boost::is_any_of(";"));
        if (parts.size() < 4) {
            cout << "解析失败：数据字段不完整" << endl;
            return;
        }

        string parsedName = parts[0];
        int deckTypeValue = 0;
        try { deckTypeValue = stoi(parts[1]); } catch(...) { deckTypeValue = +DeckType::Casual; }
        DeckType dt = (deckTypeValue == +DeckType::Standard) ? +DeckType::Standard : +DeckType::Casual;

        string charIds = parts[2];
        string cardIds = parts[3];
        // 若有第五字段为最大卡数，则可解析（可选)
        int maxLimit = 20;
        if (parts.size() >= 5) {
            try { maxLimit = stoi(parts[4]); } catch(...) { /* ignore */ }
        }

        // 询问用户给定牌组名
        string newName;
        cout << "牌组解析成功，原始牌组名: " << parsedName << endl;
        cout << "请输入导入后的牌组名称（回车使用原名）: ";
        getline(cin, newName);
        if (newName.empty()) newName = parsedName;

        Deck importedDeck(newName, dt);
        // 解析角色 id 并加入
        vector<string> charIdList;
        boost::split(charIdList, charIds, boost::is_any_of(","));
        for (const auto &cid : charIdList) {
            if (cid.empty()) continue;
            auto ch = characterDB.findCharacterById(cid);
            if (ch) importedDeck.addCharacter(ch);
        }
        // 解析卡牌 id 并加入
        vector<string> cardIdList;
        boost::split(cardIdList, cardIds, boost::is_any_of(","));
        for (const auto &cid : cardIdList) {
            if (cid.empty()) continue;
            auto c = cardDB.findCardById(cid);
            if (c) importedDeck.addCard(c);
        }
        // 尝试设置最大卡牌限制（如果类支持 setMaxCardLimit）
        // ... 若 Deck 类提供 setMaxCardLimit，可在此调用 importedDeck.setMaxCardLimit(maxLimit);

        // 更新牌组编码并加入列表 
        // 牌组已通过 addCharacter/addCard 更新其内部状态，直接加入列表
        decks.push_back(importedDeck);
        importedDeck.display();
        cout << "手动导入成功!" << endl;
        importedDeck.display();
    } catch (...) {
        cout << "手动解析失败，导入取消。" << endl;
    }
}

void GameManager::showMenu() const {
    cout << "\n=== 魔法伤痕卡牌游戏 ===" << endl;
    cout << "1. 查看所有卡牌" << endl;
    cout << "2. 查看所有角色" << endl;
    cout << "3. 创建牌组" << endl;
    cout << "4. 查看我的牌组" << endl;
    cout << "5. 查看牌组详情" << endl;
    cout << "6. 导出牌组代码" << endl;
    cout << "7. 导入牌组代码" << endl;
    cout << "8. 退出" << endl;
    cout << "9. 开始对局" << endl;
    cout << "10. 局域网联机（主机/加入）" << endl; // 新增联机选项
    cout << "选择: ";
}

void GameManager::run() {
    int choice;
    do {
        showMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                displayAllCards();
                break;
            case 2:
                displayAllCharacters();
                break;
            case 3:
                createDeck();
                break;
            case 4:
                displayDecks();
                break;
            case 5:
                displayDeckDetails();
                break;
            case 6:
                exportDeckCode();
                break;
            case 7:
                importDeckFromCode();
                break;
            case 8:
                cout << "再见!" << endl;
                break;
            case 9: { // 对局实现（编号选角、选择已有牌组、卡牌效果注册表）
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除缓冲

                struct PlayerCharState {
                    shared_ptr<Character> ch;
                    int curHP;
                    int curEnergy;
                };
                struct PlayerState {
                    string name;
                    int baseHP = 50;
                    int baseMana = 30;
                    vector<PlayerCharState> chars; // 0,1 前场；2 后场（替补）
                    vector<shared_ptr<Card>> deck;
                    vector<shared_ptr<Card>> hand;
                };

                // 选择先从已保存的牌组中选择牌组作为玩家牌库
                if (decks.empty()) {
                    cout << "没有已创建的牌组，请先创建牌组后再开始对局。" << endl;
                    break;
                }
                auto chooseDeckForPlayer = [this](const string &playerName) -> Deck* {
                    cout << playerName << " 请选择一个牌组编号：" << endl;
                    for (size_t i = 0; i < decks.size(); ++i) {
                        cout << "[" << i << "] " << decks[i].getName() << " (" << decks[i].getCardCount() << " 张)" << endl;
                    }
                    while (true) {
                        cout << "输入编号: ";
                        string s; getline(cin, s);
                        int idx = -1;
                        try { idx = stoi(s); } catch(...) { idx = -1; }
                        if (idx >= 0 && idx < (int)decks.size()) return &decks[idx];
                        cout << "无效编号，请重试。" << endl;
                    }
                };

                // 创建两个玩家并选择牌组、选角（按编号）
                auto promptSelectCharsByIndex = [this](PlayerState &p) {
                    cout << "玩家 " << p.name << " 请从列表中选择 3 个角色的编号:" << endl;
                    auto all = characterDB.getAllCharacters();
                    for (size_t i = 0; i < all.size(); ++i) cout << "[" << i << "] " << all[i]->getName() << endl;
                    for (int i = 0; i < 3; ++i) {
                        cout << "选择第" << (i+1) << "个角色编号: ";
                        string s; getline(cin, s);
                        int idx = -1; try { idx = stoi(s); } catch(...) { idx = -1; }
                        if (idx < 0 || idx >= (int)all.size()) { cout << "无效编号，重试。" << endl; --i; continue; }
                        PlayerCharState pcs; pcs.ch = all[idx]; pcs.curHP = all[idx]->getHealth(); pcs.curEnergy = (all[idx]->getEnergy()+1)/2;
                        p.chars.push_back(pcs);
                    }
                };

                PlayerState p1, p2;
                cout << "请输入玩家1 名称: "; getline(cin, p1.name); if (p1.name.empty()) p1.name="玩家1";
                Deck* d1 = chooseDeckForPlayer(p1.name);
                cout << "请输入玩家2 名称: "; getline(cin, p2.name); if (p2.name.empty()) p2.name="玩家2";
                Deck* d2 = chooseDeckForPlayer(p2.name);

                promptSelectCharsByIndex(p1);
                promptSelectCharsByIndex(p2);

                // 解析 Deck::getDeckCode() 中的卡牌 ID，并构建玩家牌库（使用 cardDB 查找）
                auto buildDeckFromDeckCode = [this](const string &deckCode, vector<shared_ptr<Card>> &outDeck) {
                    outDeck.clear();
                    try {
                        string decoded = base64::decode(deckCode);
                        size_t sep = decoded.find('|');
                        if (sep == string::npos) return;
                        string data = decoded.substr(0, sep);
                        vector<string> parts;
                        boost::split(parts, data, boost::is_any_of(";"));
                        if (parts.size() < 4) return;
                        string cardIds = parts[3];
                        vector<string> cardIdList;
                        boost::split(cardIdList, cardIds, boost::is_any_of(","));
                        for (const auto &cid : cardIdList) {
                            if (cid.empty()) continue;
                            auto c = cardDB.findCardById(cid);
                            if (c) outDeck.push_back(c);
                        }
                    } catch(...) { return; }
                };

                auto initializePlayerDeck = [&](PlayerState &p, Deck* chosenDeck) {
                    buildDeckFromDeckCode(chosenDeck->getDeckCode(), p.deck);
                    if (p.deck.empty()) {
                        auto all = cardDB.getAllCards();
                        for (const auto &c : all) p.deck.push_back(c);
                    }
                    std::random_device rd; std::mt19937 g(rd());
                    std::shuffle(p.deck.begin(), p.deck.end(), g);
                    for (int i = 0; i < 3 && !p.deck.empty(); ++i) { p.hand.push_back(p.deck.back()); p.deck.pop_back(); }
                };

                initializePlayerDeck(p1, d1);
                initializePlayerDeck(p2, d2);

                // 辅助：判断角色是否为法师（拥有除Physical外的元素）
                auto isMage = [](const shared_ptr<Character>& ch) {
                    for (auto &e : ch->getElements()) {
                        if (e != +Element::Physical) return true;
                    }
                    return false;
                };

                // 在对局内部注册卡牌效果（指针函数/回调），effect 可修改 finalDmg 或产生副作用
                unordered_map<string, function<void(PlayerState&, PlayerState&, int, bool, int, shared_ptr<Card>, int&, bool&)>> cardEffects;
                // 辅助：抽牌
                auto drawCards = [](PlayerState &p, int n) {
                    for (int i = 0; i < n && !p.deck.empty(); ++i) { p.hand.push_back(p.deck.back()); p.deck.pop_back(); }
                };
                cardEffects["Wordle"] = [](PlayerState&, PlayerState&, int, bool, int, shared_ptr<Card>, int &finalDmg, bool &) {
                    finalDmg *= 2; cout << "[效果] Wordle: 伤害翻倍！" << endl;
                };
                cardEffects["IDontcar"] = [](PlayerState&, PlayerState&, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    cout << "[效果] 窝不载乎：对手似乎被汽车鸣笛分散了注意力。" << endl;
                };
                cardEffects["madposion"] = [](PlayerState&, PlayerState&, int, bool, int, shared_ptr<Card>, int &finalDmg, bool&) {
                    finalDmg *= 3; cout << "[效果] 狂乱药水：伤害×3（简化）。" << endl;
                };
                cardEffects["organichemistry"] = [drawCards](PlayerState& owner, PlayerState&, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    drawCards(owner, 3); cout << "[效果] 魔药学：抽取最多3张牌。" << endl;
                };
                cardEffects["slowdown"] = [](PlayerState&, PlayerState& opp, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    int dec = 2; opp.baseMana = max(0, opp.baseMana - dec); cout << "[效果] 缓慢药水：对手基地魔力 -" << dec << "。" << endl;
                };
                cardEffects["Timeelder"] = [](PlayerState&, PlayerState& opp, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    if (!opp.hand.empty()) { cout << "[效果] 时空限速：对手弃掉手牌 " << opp.hand.back()->getName() << "。" << endl; opp.hand.pop_back(); }
                };
                cardEffects["LGBTQ"] = [](PlayerState& owner, PlayerState&, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    owner.baseMana += 1000; cout << "[效果] 多彩药水：本回合获得属性适配（简化）。" << endl;
                };
                cardEffects["Lazarus,Arise!"] = [](PlayerState& owner, PlayerState&, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    owner.baseHP += 5; cout << "[效果] 起尸：基地回复5生命（简化）。" << endl;
                };
                cardEffects["DontForgotMe"] = [](PlayerState& owner, PlayerState& opp, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    int move = min(8, (int)opp.deck.size());
                    for (int i = 0; i < move; ++i) { owner.deck.push_back(opp.deck.back()); opp.deck.pop_back(); }
                    cout << "[效果] 瓶装记忆：将对手牌库顶最多 " << move << " 张牌移入我的牌库（简化）。" << endl;
                };
                cardEffects["TheCardLetMeWin"] = [](PlayerState&, PlayerState& opp, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    for (int i = 0; i < 2 && !opp.deck.empty(); ++i) opp.deck.pop_back();
                    for (int i = 0; i < 2 && !opp.deck.empty(); ++i) opp.deck.erase(opp.deck.begin());
                    cout << "[效果] 记忆屏蔽：摧毁对手牌库顶/底各2张（简化）。" << endl;
                };
                cardEffects["TheCardLetYouLose"] = [](PlayerState& owner, PlayerState& opp, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    for (int i = 0; i < 2 && !owner.deck.empty(); ++i) owner.deck.pop_back();
                    for (int i = 0; i < 2 && !owner.deck.empty(); ++i) owner.deck.erase(owner.deck.begin());
                    for (int i = 0; i < 2 && !opp.deck.empty(); ++i) opp.deck.pop_back();
                    for (int i = 0; i < 2 && !opp.deck.empty(); ++i) opp.deck.erase(opp.deck.begin());
                    if (owner.deck.empty()) owner.baseHP = 0;
                    cout << "[效果] 记忆摧毁：双方顶底各2张，被激活后若你的牌库为空你输（简化）。" << endl;
                };
                cardEffects["whAt"] = [](PlayerState&, PlayerState& opp, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    if (!opp.deck.empty()) { cout << "[效果] 你说啥？：摧毁对手一张牌 " << opp.deck.back()->getName() << "（顶）。" << endl; opp.deck.pop_back(); }
                };
                cardEffects["balance"] = [drawCards](PlayerState& owner, PlayerState&, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    int n = owner.hand.size(); owner.hand.clear(); drawCards(owner, n); cout << "[效果] 平衡：弃手并抽等量的牌（简化）。" << endl;
                };
                cardEffects["TearAll"] = [](PlayerState&, PlayerState& opp, int, bool, int, shared_ptr<Card>, int&, bool&) {
                    opp.deck.clear(); cout << "[效果] 遗忘灵药：摧毁对手牌库（简化）。" << endl;
                };

                // 辅助：替补上阵（当前场 idx 位置死亡，用后场替补到该位置）
                auto tryReplaceDead = [](PlayerState &p, int deadIndex) {
                    if (deadIndex < 0 || deadIndex > 1) return;
                    if (p.chars.size() >= 3) {
                        if (p.chars.size() == 3) {
                            auto replaced = p.chars[2];
                            p.chars[deadIndex] = replaced;
                            p.chars.pop_back();
                            cout << p.name << " 的后场角色已替补到前场位置 " << deadIndex+1 << "。" << endl;
                        }
                    }
                };

                // 辅助：应用伤害到目标（角色或基地），并处理溢出到基地与替补
                auto applyDamageToChar = [&](PlayerState &owner, int idx, int dmg, bool isMagic) {
                    if (idx < 0 || idx > 1) return;
                    if (idx >= (int)owner.chars.size()) return;
                    PlayerCharState &t = owner.chars[idx];
                    if (isMagic && isMage(t.ch)) {
                        int energyTaken = min(t.curEnergy, dmg);
                        t.curEnergy -= energyTaken;
                        dmg -= energyTaken;
                    }
                    if (dmg > 0) t.curHP -= dmg;
                    if (t.curHP <= 0) {
                        int overflow = -t.curHP;
                        cout << owner.name << " 的角色 " << t.ch->getName() << " 被击败！" << endl;
                        bool hasReserve = owner.chars.size() == 3;
                        if (hasReserve) {
                            owner.chars[idx] = owner.chars[2];
                            owner.chars.pop_back();
                        } else {
                            owner.chars[idx].curHP = 0;
                        }
                        if (overflow > 0) { owner.baseHP -= overflow; cout << owner.name << " 的基地受到溢出伤害 " << overflow << " 点！" << endl; }
                    }
                };

                auto applyDamageToBase = [&](PlayerState &owner, int dmg) { owner.baseHP -= dmg; };

                auto checkWinner = [&](const PlayerState &a, const PlayerState &b) -> int {
                    if (a.baseHP <= 0) return 2;
                    if (b.baseHP <= 0) return 1;
                    return 0;
                };

                // 每回合处理与出牌循环
                int turn = 1;
                int active = 0; // 0 -> p1, 1 -> p2
                bool running = true;
                while (running) {
                    PlayerState &cur = (active == 0) ? p1 : p2;
                    PlayerState &opp = (active == 0) ? p2 : p1;

                    cout << "\n=== 回合 " << turn << " - " << cur.name << " 的回合开始 ===" << endl;
                    if (!cur.deck.empty()) { cur.hand.push_back(cur.deck.back()); cur.deck.pop_back(); cout << cur.name << " 抽了1张牌。" << endl; }
                    else cout << cur.name << " 的牌库已空，无法抽牌。" << endl;

                    cur.baseMana = min(30, cur.baseMana + 5);
                    for (auto &pcs : cur.chars) { int maxE = pcs.ch->getEnergy(); pcs.curEnergy = min(maxE, pcs.curEnergy + 5); }

                    while (true) {
                        auto showPlayerState = [](const PlayerState &p) {
                            cout << "\n玩家: " << p.name << " | 基地生命: " << p.baseHP << " | 基地魔力: " << p.baseMana << endl;
                            cout << "前场角色:" << endl;
                            for (int i = 0; i < (int)p.chars.size() && i < 2; ++i) {
                                const auto &pcs = p.chars[i];
                                cout << " [" << i << "] " << pcs.ch->getName() << " (HP: " << pcs.curHP << "/" << pcs.ch->getHealth()
                                     << ", MP: " << pcs.curEnergy << "/" << pcs.ch->getEnergy() << ")" << endl;
                            }
                            if (p.chars.size() == 3) {
                                const auto &r = p.chars[2];
                                cout << " 后场替补: " << r.ch->getName() << " (HP: " << r.curHP << "/" << r.ch->getHealth()
                                     << ", MP: " << r.curEnergy << "/" << r.ch->getEnergy() << ")" << endl;
                            } else cout << " 无后场替补" << endl;
                            cout << "手牌(" << p.hand.size() << "): ";
                            for (int i = 0; i < (int)p.hand.size(); ++i) cout << "[" << i << "]" << p.hand[i]->getName() << " ";
                            cout << endl;
                        };

                        showPlayerState(cur);
                        showPlayerState(opp);

                        cout << "\n操作：p 出牌；e 结束回合；q 退出对局。输入操作字母: ";
                        string op; getline(cin, op);
                        if (op == "q" || op == "Q") { cout << "对局提前结束。" << endl; running = false; break; }
                        if (op == "e" || op == "E") { cout << "结束回合。" << endl; break; }
                        if (op == "p" || op == "P") {
                            if (cur.hand.empty()) { cout << "手牌为空，无法出牌。" << endl; continue; }
                            cout << "选择出牌的手牌索引: ";
                            string idxs; getline(cin, idxs);
                            int hidx = -1;
                            try { hidx = stoi(idxs); } catch(...) { hidx = -1; }
                            if (hidx < 0 || hidx >= (int)cur.hand.size()) { cout << "无效手牌索引。" << endl; continue; }
                            auto card = cur.hand[hidx];
                            bool isPhysical = false;
                            for (auto &e : card->getElements()) if (e == +Element::Physical) isPhysical = true;
                            cout << "选择使用该牌的前场角色索引(0或1): ";
                            string sidx; getline(cin, sidx);
                            int charIdx = -1; try { charIdx = stoi(sidx); } catch(...) { charIdx = -1; }
                            if (charIdx < 0 || charIdx > 1 || charIdx >= (int)cur.chars.size()) { cout << "无效角色索引。" << endl; continue; }
                            auto &actor = cur.chars[charIdx];
                            bool actorIsMage = isMage(actor.ch);
                            if (!actorIsMage && !isPhysical) { cout << "普通人只能使用物理属性的牌，无法打出该牌。" << endl; continue; }
                            cout << "选择目标：输入 t0 或 t1 指对方对应前场，输入 b 指对方基地: ";
                            string target; getline(cin, target);
                            bool targetIsBase = false; int targetIdx = -1;
                            if (target == "b" || target == "B") targetIsBase = true;
                            else if (target == "t0" || target == "t1") { targetIdx = (target == "t0") ? 0 : 1; if (targetIdx >= (int)opp.chars.size()) { cout << "对方该前场位置没有角色，无法作为目标。" << endl; continue; } }
                            else { cout << "无效目标指示。" << endl; continue; }

                            int cost = card->getCost(); if (isPhysical) cost = 0;
                            int remainingCost = cost;
                            if (actorIsMage && cost > 0) {
                                int fromChar = min(actor.curEnergy, remainingCost); actor.curEnergy -= fromChar; remainingCost -= fromChar;
                                int fromBase = min(cur.baseMana, remainingCost); cur.baseMana -= fromBase; remainingCost -= fromBase;
                                if (remainingCost > 0) { cout << "魔力不足，使用生命支付剩余费用: " << remainingCost << " 点（直接扣角色生命）。" << endl; actor.curHP -= remainingCost; remainingCost = 0; }
                            }

                            int baseDmg = max(1, card->getCost());
                            bool elementMatch = false;
                            for (auto &ce : card->getElements()) if (actor.ch->hasElement(ce)) { elementMatch = true; break; }
                            int finalDmg = baseDmg * (elementMatch ? 2 : 1);
                            bool dmgIsMagic = !isPhysical;

                            // 先执行卡牌效果（若注册）
                            if (cardEffects.count(card->getId())) {
                                try { cardEffects[card->getId()](cur, opp, charIdx, targetIsBase, targetIdx, card, finalDmg, dmgIsMagic); } catch(...) {}
                            }

                            cout << actor.ch->getName() << " 使用 " << card->getName() << " 对 ";
                            if (targetIsBase) cout << opp.name << " 的基地"; else cout << opp.chars[targetIdx].ch->getName();
                            cout << " 造成 " << finalDmg << (dmgIsMagic ? " 魔法伤害" : " 物理伤害") << "（已支付消耗）。" << endl;

                            if (targetIsBase) {
                                applyDamageToBase(opp, finalDmg);
                                cout << opp.name << " 的基地剩余生命: " << opp.baseHP << endl;
                            } else {
                                applyDamageToChar(opp, targetIdx, finalDmg, dmgIsMagic);
                            }

                            cur.hand.erase(cur.hand.begin() + hidx);

                            int win = checkWinner(p1, p2);
                            if (win != 0) { 
								if (win == 1) cout << p1.name << " 获胜！" << endl; 
								else cout << p2.name << " 获胜！" << endl; 
								running = false; 
								break; 
							}
                        } else cout << "未知操作，请重试。" << endl;
                    } // 回合内行动循环结束

					if (!running) break;

					// 回合切换
					active = 1 - active;
					++turn;
				} // 对局主循环结束

				cout << "对局结束，返回主菜单。" << endl;
			} break;

			case 10: { // 局域网联机（主机/加入） - 简化的对战同步 + 表情原型（Windows 下可用）
				cin.ignore(numeric_limits<streamsize>::max(), '\n');

				cout << "局域网联机模式：选择 1 主机，2 加入，其他 取消: ";
				string mode;
				getline(cin, mode);
				if (mode != "1" && mode != "2") {
					cout << "取消联机。" << endl;
					break;
				}

#if defined(_WIN32) || defined(_WIN64)
				WSADATA wsaData;
				if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
					cout << "WSAStartup 失败，无法使用网络。" << endl;
					break;
				}
				using socket_t = SOCKET;
#else
				cout << "当前平台未实现局域网联机（仅 Windows 支持），返回主菜单。" << endl;
				break;
#endif
				const int BUF = 4096;
				atomic<bool> netRunning{true};
				queue<string> recvQ;
				mutex qMutex;
				condition_variable qCv;
				auto enqueue = [&](string m){
					lock_guard<mutex> lk(qMutex);
					recvQ.push(move(m));
					qCv.notify_one();
				};
				auto dequeueAll = [&]()->vector<string>{
					vector<string> out;
					lock_guard<mutex> lk(qMutex);
					while(!recvQ.empty()){ out.push_back(recvQ.front()); recvQ.pop(); }
					return out;
				};

				socket_t conn = 0;
				bool isHost = (mode == "1");

#if defined(_WIN32) || defined(_WIN64)
				if (isHost) {
					cout << "主机：输入监听端口（默认4000）: ";
					string ps; getline(cin, ps); int port=4000; try{ if(!ps.empty()) port=stoi(ps); }catch(...) {}
					SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					if (listenSock == INVALID_SOCKET) { cout << "创建监听失败: " << WSAGetLastError() << endl; WSACleanup(); break; }
					sockaddr_in svc{}; svc.sin_family = AF_INET; svc.sin_addr.s_addr = INADDR_ANY; svc.sin_port = htons((unsigned short)port);
					if (bind(listenSock, (SOCKADDR*)&svc, sizeof(svc)) == SOCKET_ERROR) { cout << "bind 失败: " << WSAGetLastError() << endl; closesocket(listenSock); WSACleanup(); break; }
					if (listen(listenSock,1) == SOCKET_ERROR) { cout << "listen 失败: " << WSAGetLastError() << endl; closesocket(listenSock); WSACleanup(); break; }
					cout << "等待连接，端口 " << port << " ..." << endl;
					SOCKET client = accept(listenSock, nullptr, nullptr);
					if (client == INVALID_SOCKET) { cout << "accept 失败: " << WSAGetLastError() << endl; closesocket(listenSock); WSACleanup(); break; }
					conn = client; closesocket(listenSock);
				} else {
					cout << "加入：输入主机地址（默认127.0.0.1）: ";
					string host; getline(cin, host); if (host.empty()) host="127.0.0.1";
					cout << "输入端口（默认4000）: ";
					string ps; getline(cin, ps); int port=4000; try{ if(!ps.empty()) port=stoi(ps); }catch(...) {}
					SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					if (sock == INVALID_SOCKET) { cout << "socket 失败: " << WSAGetLastError() << endl; WSACleanup(); break; }
					sockaddr_in serv{}; serv.sin_family = AF_INET; inet_pton(AF_INET, host.c_str(), &serv.sin_addr); serv.sin_port = htons((unsigned short)port);
					cout << "尝试连接..." << endl;
					if (connect(sock, (SOCKADDR*)&serv, sizeof(serv)) == SOCKET_ERROR) { cout << "connect 失败: " << WSAGetLastError() << endl; closesocket(sock); WSACleanup(); break; }
					conn = sock;
				}
#endif

				// 启动接收线程
				thread recvThread([&](){
					char buf[BUF];
					while(netRunning){
#if defined(_WIN32) || defined(_WIN64)
						int r = recv(conn, buf, BUF-1, 0);
#else
						int r = recv(conn, buf, BUF-1, 0);
#endif
						if (r <= 0) { netRunning = false; qCv.notify_one(); break; }
						buf[r]=0;
						// 支持粘包：逐行分割
						string s(buf);
						size_t pos=0;
						while(true){
							size_t nl = s.find('\n', pos);
							if (nl==string::npos) { string tail = s.substr(pos); if(!tail.empty()) enqueue(tail); break; }
							string line = s.substr(pos, nl-pos);
							enqueue(line);
							pos = nl+1;
						}
					}
				});

				// 简单握手：交换 NAME
				cout << "请输入你的名称: ";
				string myName; getline(cin, myName); if (myName.empty()) myName = (isHost ? "Host" : "Client");
				auto sendLine = [&](const string &m){
#if defined(_WIN32) || defined(_WIN64)
					send(conn, m.c_str(), (int)m.size(), 0);
#endif
				};
				sendLine(string("NAME;")+myName+"\n");
				string theirName = "对手";
				// 等待短时间看是否收到 NAME
				{
					unique_lock<mutex> lk(qMutex);
					if (qCv.wait_for(lk, chrono::seconds(2), [&]{ return !recvQ.empty(); })) {
						auto v = dequeueAll();
						for (auto &msg : v) if (msg.rfind("NAME;",0)==0) theirName = msg.substr(5);
					}
				}
				cout << "已连接: " << theirName << endl;

				// 选择牌组与角色（仅本地选择）
				if (decks.empty()) { cout << "没有牌组，取消联机。" << endl; netRunning=false; if (recvThread.joinable()) recvThread.join(); closesocket(conn); WSACleanup(); break; }
				displayDecks();
				cout << "选择你的牌组编号: ";
				string ds; getline(cin, ds); int didx = 0; try{ didx=stoi(ds); }catch(...){ didx=0; } if (didx<0||didx>=(int)decks.size()) didx=0;
				Deck* chosen = &decks[didx];
				// 发送 DECKCODE（供对端参考）
				sendLine(string("DECKCODE;")+chosen->getDeckCode()+"\n");

				// 本地玩家状态结构（简化复制）
				struct NChar { shared_ptr<Character> ch; int hp; int energy; };
				struct NPlayer { string name; int baseHP=50; int baseMana=30; vector<NChar> chars; vector<shared_ptr<Card>> deck; vector<shared_ptr<Card>> hand; };
				NPlayer local, remote; local.name = myName; remote.name = theirName;

				// 构建本地牌库并抽初始手牌
				{ vector<shared_ptr<Card>> tmp; 
				  // reuse local helper buildDeckFromDeckCode if available; fallback to cardDB
				  try { string code = chosen->getDeckCode(); string decoded = base64::decode(code); size_t sep = decoded.find('|'); if (sep!=string::npos){ string data=decoded.substr(0,sep); vector<string> parts; boost::split(parts,data,boost::is_any_of(";")); if (parts.size()>=4){ vector<string> ids; boost::split(ids, parts[3], boost::is_any_of(",")); for (auto &id:ids){ if (id.empty()) continue; auto c = cardDB.findCardById(id); if (c) tmp.push_back(c);} } } } catch(...){} 
				  if (tmp.empty()) tmp = cardDB.getAllCards();
				  local.deck = tmp;
				  // shuffle
				  std::random_device rd; std::mt19937 g(rd()); std::shuffle(local.deck.begin(), local.deck.end(), g);
				  for (int i=0;i<3 && !local.deck.empty();++i){ local.hand.push_back(local.deck.back()); local.deck.pop_back(); }
				}

				// 选择3个角色
				auto allChars = characterDB.getAllCharacters();
				cout << "请选择3个角色编号（按回车确认每个）:" << endl;
				for (size_t i=0;i<allChars.size();++i) cout << "["<<i<<"] "<<allChars[i]->getName() << endl;
				for (int k=0;k<3;++k){
					cout << "第" << k+1 << "个: ";
					string cs; getline(cin, cs); int ci=0; try{ci=stoi(cs);}catch(...){ci=0;}
					if (ci<0||ci>=(int)allChars.size()) ci=0;
					NChar nc; nc.ch = allChars[ci]; nc.hp = nc.ch->getHealth(); nc.energy = (nc.ch->getEnergy()+1)/2; local.chars.push_back(nc);
				}
				// 发送本地角色 id 列表
				{ string ids; for (size_t i=0;i<local.chars.size();++i){ if (i) ids+=","; ids += local.chars[i].ch->getId(); } sendLine(string("CHARS;")+ids+"\n"); }

				// 等待对端的 CHARS（最多等待 10 秒），确保 remote.chars 已初始化
				{
					auto start = chrono::steady_clock::now();
					bool gotChars = false;
					while (!gotChars && chrono::steady_clock::now() - start < chrono::seconds(10) && netRunning) {
						// 先快速抓取队列中的消息
						auto msgs = dequeueAll();
						for (auto &m : msgs) {
							if (m.rfind("CHARS;", 0) == 0) {
								vector<string> arr; boost::split(arr, m.substr(6), boost::is_any_of(","));
								for (auto &id : arr) {
									auto ch = characterDB.findCharacterById(id);
									if (ch) { NChar nc; nc.ch = ch; nc.hp = ch->getHealth(); nc.energy = (ch->getEnergy()+1)/2; remote.chars.push_back(nc); }
								}
								gotChars = true;
							} else if (m.rfind("DECKCODE;", 0) == 0) {
								// 可选：记录对端牌组码或临时解析对端牌库（这里简化忽略）
							}
						}
						if (!gotChars) {
							unique_lock<mutex> lk(qMutex);
							qCv.wait_for(lk, chrono::milliseconds(200));
						}
					}
					if (!gotChars) {
						cout << "警告：未在超时内收到对手角色信息，继续游戏但无法选择对方前场目标。" << endl;
					} else {
						cout << "已接收对手角色信息，准备开始对战。" << endl;
					}
				}
				
				// 工具：应用对方 PLAY 到本地（简化伤害逻辑）
				auto applyRemotePlay = [&](const string &m){
					if (m.rfind("PLAY;",0)==0){
						vector<string> p; boost::split(p,m,boost::is_any_of(";"));
						if (p.size()>=4){
							string cid = p[1]; int actor = stoi(p[2]); string target = p[3];
							auto cardptr = cardDB.findCardById(cid);
							if (!cardptr) return;
							bool isPhysical = false; for (auto &e : cardptr->getElements()) if (e==+Element::Physical) isPhysical=true;
							int baseD = max(1, cardptr->getCost());
							int finalD = baseD;
							// 不考虑元素匹配（远端 actor 元素未知），直接应用
							bool dmgMagic = !isPhysical;
							if (target=="b"){ local.baseHP -= finalD; cout << "\n[对方] 对你基地造成 " << finalD << " 点伤害\n"; }
							else if (target=="t0"||target=="t1"){ int tidx = (target=="t0")?0:1; if (tidx < (int)local.chars.size()){ auto &tc = local.chars[tidx]; if (dmgMagic){ int et = min(tc.energy, finalD); tc.energy -= et; finalD -= et; } if (finalD>0) tc.hp -= finalD; cout << "\n[对方] 攻击了你的 " << tc.ch->getName() << "\n"; } }
						}
					} else if (m.rfind("EMOJI;",0)==0){
						cout << "\n[对方表情] " << m.substr(6) << endl;
					} else if (m.rfind("DECKCODE;",0)==0){
						// ignore or store
					} else if (m.rfind("CHARS;",0)==0){
						vector<string> arr; boost::split(arr, m.substr(6), boost::is_any_of(","));
						for (auto &id : arr){ auto ch = characterDB.findCharacterById(id); if (ch){ NChar nc; nc.ch = ch; nc.hp = ch->getHealth(); nc.energy = (ch->getEnergy()+1)/2; remote.chars.push_back(nc);} }
					} else if (m=="ENDTURN"){ /* treat as turn end indicator */ }
				};

				// 简化的回合控制：主机先手
				bool myTurn = isHost;
				cout << "网络对战开始，主机先手。" << endl;
				while(netRunning){
					// 先处理收到的消息
					auto msgs = dequeueAll();
					for (auto &mm : msgs) { if (!mm.empty()) applyRemotePlay(mm); }
					// 检查胜利
					if (local.baseHP<=0 || remote.baseHP<=0){ if (local.baseHP<=0) cout << "你被击败。" << endl; else cout << "你获胜！" << endl; break; }
					if (!myTurn){
						// 等待对方动作: block a bit for new messages
						unique_lock<mutex> lk(qMutex);
						qCv.wait_for(lk, chrono::milliseconds(300));
						continue;
					}
					// 我的回合：允许出牌/发表情/结束回合/退出
					cout << "\n你的回合：p 出牌；/emoji 文本 发送表情；e 结束回合；q 退出: ";
					string op; getline(cin, op);
					if (op=="q"){ netRunning=false; break; }
					if (op.rfind("/emoji",0)==0){ string em = op.size()>6?op.substr(7):"🙂"; sendLine(string("EMOJI;")+em+"\n"); cout << "[已发送表情] " << em << endl; continue; }
					if (op=="e"){ sendLine(string("ENDTURN\n")); myTurn=false; continue; }
					if (op=="p"){
						// 显示手牌
						for (int i=0;i<(int)local.hand.size();++i) cout << "["<<i<<"]"<<local.hand[i]->getName()<<" ";
						cout << "\n选择手牌索引: ";
						string hs; getline(cin, hs); int hi=-1; try{ hi=stoi(hs);}catch(...){hi=-1;}
						if (hi<0 || hi>=(int)local.hand.size()){ cout << "无效索引\n"; continue; }
						auto card = local.hand[hi];
						cout << "选择角色索引(0或1): "; string as; getline(cin,as); int ai=0; try{ai=stoi(as);}catch(...){ai=0;}
						if (ai<0 || ai>=(int)local.chars.size()){ cout << "无效角色\n"; continue; }
						cout << "选择目标：t0/t1/b: "; string tgt; getline(cin,tgt);
						// 本地应用
						bool isPhysical=false; for (auto &e : card->getElements()) if (e==+Element::Physical) isPhysical=true;
						int baseD = max(1, card->getCost()); int finalD = baseD;
						bool dmgMagic = !isPhysical;
						if (tgt=="b"){ remote.baseHP -= finalD; cout << "对对手基地造成 " << finalD << " 点伤害\n"; }
						else if (tgt=="t0"||tgt=="t1"){ int tidx = (tgt=="t0")?0:1; if (tidx < (int)remote.chars.size()){ auto &tc = remote.chars[tidx]; if (dmgMagic){ int et=min(tc.energy,finalD); tc.energy -= et; finalD -= et; } if (finalD>0) tc.hp -= finalD; cout << "对对手前场造成伤害\n"; } }
						// 发送 PLAY
						string msg = string("PLAY;") + card->getId() + ";" + to_string(ai) + ";" + tgt + "\n";
						sendLine(msg);
						local.hand.erase(local.hand.begin()+hi);
						continue;
					}
				} // net loop

				// 清理
				netRunning=false;
				qCv.notify_one();
				if (recvThread.joinable()) recvThread.join();
#if defined(_WIN32) || defined(_WIN64)
				closesocket(conn);
				WSACleanup();
#endif
				cout << "退出联机。" << endl;
			} break;

			default:
				cout << "无效选择，请重试。" << endl;
		}

		// 清除输入缓冲区
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

	} while (choice != 8);
}
