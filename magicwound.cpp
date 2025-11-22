#include "magicwound.h"

using namespace std;

// CRC32 实现
namespace crc32 {
    static const uint32_t crc32_table[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
    };

    uint32_t calculate(const std::string& input) {
        uint32_t crc = 0xFFFFFFFF;
        for (char c : input) {
            crc = (crc >> 8) ^ crc32_table[(crc ^ c) & 0xFF];
        }
        return crc ^ 0xFFFFFFFF;
    }

    std::string generate_checksum(const std::string& input) {
        uint32_t crc = calculate(input);
        stringstream ss;
        ss << hex << setw(8) << setfill('0') << crc;
        return ss.str().substr(0, 4);
    }
}

// Base64 实现
namespace base64 {
    static const string chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    string encode(const string &input) {
        string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        size_t in_len = input.size();
        const char* bytes_to_encode = input.c_str();

        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; (i <4) ; i++)
                    ret += chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += chars[char_array_4[j]];

            while((i++ < 3))
                ret += '=';
        }

        return ret;
    }

    string decode(const string &input) {
        size_t in_len = input.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        string ret;

        while (in_len-- && ( input[in_] != '=') && is_base64(input[in_])) {
            char_array_4[i++] = input[in_]; in_++;
            if (i ==4) {
                for (i = 0; i <4; i++)
                    char_array_4[i] = chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j <4; j++)
                char_array_4[j] = 0;

            for (j = 0; j <4; j++)
                char_array_4[j] = chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }

        return ret;
    }
}

// Character 实现
Character::Character(const string& id, const string& name, 
                   const vector<Element>& elements, int health, int energy,
                   const string& ability, const string& description)
    : id(id), name(name), elements(elements), health(health), 
      energy(energy), ability(ability), description(description) {}

bool Character::hasElement(Element element) const {
    return find(elements.begin(), elements.end(), element) != elements.end();
}

void Character::display() const {
    cout << "人物: " << name << endl;
    cout << "元素: ";
    for (const auto& element : elements) {
        cout << elementToString(element) << " ";
    }
    cout << endl;
    cout << "生命值: " << health << endl;
    cout << "能量: " << energy << endl;
    cout << "能力: " << ability << endl;
    cout << "描述: " << description << endl;
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
    cout << "卡牌: " << name << endl;
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
        case +Rarity::Funny: return "娱乐";
        default: return "未知";
    }
}

// Deck 实现
Deck::Deck(const string& name, DeckType type) : name(name), deckType(type), maxCardLimit(20) {
    updateDeckCode();
}

void Deck::addCard(const shared_ptr<Card>& card) {
    // 检查套牌类型限制
    if (deckType == +DeckType::Standard && card->getRarity() == +Rarity::Funny) {
        cout << "标准套牌不能携带娱乐稀有度的卡牌: " << card->getName() << endl;
        return;
    }
    
    // 检查套牌数量限制
    if (cards.size() >= maxCardLimit) {
        cout << "套牌已达到最大卡牌数量限制 (" << maxCardLimit << "张)" << endl;
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
        cout << "套牌最多只能有3个人物" << endl;
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
    cout << "\n=== 套牌详情 ===" << endl;
    cout << "套牌名称: " << name << endl;
    cout << "套牌类型: " << deckTypeToString() << endl;
    cout << "卡牌数量: " << cards.size() << "/" << maxCardLimit << endl;
    cout << "人物数量: " << characters.size() << "/3" << endl;
    cout << "套牌代码: " << deckCode << endl;
    
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
    
    cout << "人物列表:" << endl;
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

// 修改shuffle方法
void Deck::shuffle() {
    random_device rd;
    mt19937 g(rd());
    // 使用std::前缀明确调用shuffle
    std::shuffle(cards.begin(), cards.end(), g);
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
        
        // 解析套牌数据
        istringstream iss(dataPart);
        string deckName, deckTypeStr;
        getline(iss, deckName, ';');
        getline(iss, deckTypeStr, ';');
        
        this->name = deckName;
        
        // 修复枚举初始化问题
        int deckTypeValue = stoi(deckTypeStr);
        if (deckTypeValue == +DeckType::Standard) {
            this->deckType = +DeckType::Standard;
        } else {
            this->deckType = +DeckType::Casual;
        }
        
        cards.clear();
        characters.clear();
        
        // 读取人物ID
        string characterIds;
        getline(iss, characterIds, ';');
        istringstream charStream(characterIds);
        string charId;
        while (getline(charStream, charId, ',')) {
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
        istringstream cardStream(cardIds);
        string cardId;
        while (getline(cardStream, cardId, ',')) {
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
        
        // 读取套牌限制
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
    // 构建套牌数据字符串
    stringstream data;
    data << name << ";";
    data << +deckType << ";";
    
    // 人物ID
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
    
    // 套牌限制
    data << maxCardLimit << ";";
    
    string dataStr = data.str();
    string checksum = crc32::generate_checksum(dataStr);
    
    // 将数据Base64编码
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
        case +DeckType::Standard: return "标准套牌";
        case +DeckType::Casual: return "娱乐套牌";
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
        "治疗", "消耗5点魔力，指定我人或物获得5点生命值。"
    ));
    allCharacters.push_back(make_shared<Character>(
        "neko", "三金", 
        vector<Element>{+Element::Wind}, 20, 25,
        "吹飞", "消耗10点魔力，选择一项：指定一个对方目标下场；或令一个效果消失。"
    ));
    allCharacters.push_back(make_shared<Character>(
        "soybeanmilk", "江源", 
        vector<Element>{+Element::Light}, 20, 20,
        "恢复", "消耗10点魔力将场上存在的其他人或魔物状态恢复至上回合结束时。（第二回合解锁）"
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
        vector<Element>{+Element::Water}, 10, Rarity::Mythic,
        "本局对战中，你的药水魔力消耗减少（2）。随机获取3张药水。"
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
    cout << "=== 所有人物 ===" << endl;
    for (const auto& character : characterDB.getAllCharacters()) {
        character->display();
    }
}

void GameManager::createDeck() {
    string deckName;
    cout << "请输入套牌名称: ";
    cin.ignore();
    getline(cin, deckName);
    
    // 选择套牌类型
    cout << "选择套牌类型:" << endl;
    cout << "1. 标准套牌 (不能携带娱乐稀有度卡牌)" << endl;
    cout << "2. 娱乐套牌 (可以携带所有卡牌)" << endl;
    cout << "选择: ";
    
    int typeChoice;
    cin >> typeChoice;
    cin.ignore();  // 清除换行符
    
    DeckType deckType = (typeChoice == 1) ? +DeckType::Standard : +DeckType::Casual;
    Deck newDeck(deckName, deckType);
    
    // 选择人物
    cout << "\n选择3个人物 (输入人物名称):" << endl;
    displayAllCharacters();
    
    for (int i = 0; i < 3; i++) {
        string characterName;
        cout << "选择第 " << (i + 1) << " 个人物: ";
        getline(cin, characterName);
        
        auto character = characterDB.findCharacter(characterName);
        if (character) {
            newDeck.addCharacter(character);
            cout << "已添加人物: " << characterName << endl;
        } else {
            cout << "未找到人物: " << characterName << endl;
            i--; // 重新选择
        }
    }
    
    // 选择卡牌
    cout << "\n选择要添加到套牌的卡牌 (输入卡牌名称，输入'done'结束):" << endl;
    
    // 根据套牌类型过滤卡牌
    vector<shared_ptr<Card>> availableCards;
    if (deckType == +DeckType::Standard) {
        // 标准套牌不能有Funny稀有度卡牌
        auto allCards = cardDB.getAllCards();
        copy_if(allCards.begin(), allCards.end(), back_inserter(availableCards),
            [](const shared_ptr<Card>& card) {
                return card->getRarity() != +Rarity::Funny;
            });
    } else {
        availableCards = cardDB.getAllCards();
    }
    
    for (const auto& card : availableCards) {
        cout << "- " << card->getName() << " (" << (card->getType() == +CardType::Creature ? "生物" : "法术") 
                  << ", " << card->getCost() << ", " << card->rarityToString(card->getRarity()) << ")" << endl;
    }
    
    string cardName;
    while (true) {
        cout << "输入卡牌: ";
        getline(cin, cardName);
        
        if (cardName == "done") break;
        
        auto card = cardDB.findCard(cardName);
        if (card) {
            // 检查套牌扩容卡
            if (card->getId() == "deckexpander") {
                newDeck.setMaxCardLimit(newDeck.getMaxCardLimit() + 5);
                cout << "套牌扩容! 最大卡牌数量增加到: " << newDeck.getMaxCardLimit() << endl;
            }
            newDeck.addCard(card);
            cout << "已添加卡牌: " << cardName << " (" << newDeck.getCardCount() << "/" << newDeck.getMaxCardLimit() << ")" << endl;
        } else {
            cout << "未找到卡牌: " << cardName << endl;
        }
    }
    
    // 检查套牌是否有效
    if (newDeck.isValid()) {
        decks.push_back(newDeck);
        cout << "套牌创建成功!" << endl;
    } else {
        cout << "套牌无效! 需要至少20张卡牌和3个人物。" << endl;
        cout << "当前: " << newDeck.getCardCount() << "张卡牌, " << newDeck.getCharacterCount() << "个人物" << endl;
    }
}

void GameManager::displayDecks() const {
    cout << "=== 我的套牌 ===" << endl;
    for (size_t i = 0; i < decks.size(); ++i) {
        string validStatus = decks[i].isValid() ? "有效" : "无效";
        cout << i + 1 << ". " << decks[i].getName() 
                  << " (" << decks[i].getCardCount() << " 张卡牌, " 
                  << decks[i].getCharacterCount() << " 个人物) - " << validStatus << endl;
    }
}

void GameManager::displayDeckDetails() const {
    if (decks.empty()) {
        cout << "没有套牌可显示。" << endl;
        return;
    }
    
    displayDecks();
    cout << "选择套牌编号: ";
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
        cout << "没有套牌可导出" << endl;
        return;
    }
    
    displayDecks();
    cout << "选择要导出代码的套牌编号: ";
    int choice;
    cin >> choice;
    
    if (choice > 0 && choice <= static_cast<int>(decks.size())) {
        cout << "套牌代码: " << decks[choice - 1].getDeckCode() << endl;
        cout << "请保存此代码以备后续导入。" << endl;
    } else {
        cout << "无效选择" << endl;
    }
}

void GameManager::importDeckFromCode() {
    string deckCode;
    cout << "请输入套牌代码: ";
    cin.ignore();
    getline(cin, deckCode);
    
    if (!Deck::isValidDeckCode(deckCode)) {
        cout << "无效的套牌代码!" << endl;
        return;
    }
    
    Deck newDeck("导入的套牌");
    if (newDeck.importFromDeckCode(deckCode, cardDB.getAllCards(), characterDB.getAllCharacters())) {
        string newName;
        cout << "套牌导入成功! 请输入新的套牌名称: ";
        getline(cin, newName);
        
        // 创建一个新的套牌对象并添加到列表中
        Deck importedDeck(newName, newDeck.getDeckType());
        importedDeck.importFromDeckCode(deckCode, cardDB.getAllCards(), characterDB.getAllCharacters());
        decks.push_back(importedDeck);
        
        cout << "套牌导入完成!" << endl;
        importedDeck.display();
    } else {
        cout << "套牌导入失败!" << endl;
    }
}

void GameManager::showMenu() const {
    cout << "\n=== 魔法创伤卡牌游戏 ===" << endl;
    cout << "1. 查看所有卡牌" << endl;
    cout << "2. 查看所有人物" << endl;
    cout << "3. 创建套牌" << endl;
    cout << "4. 查看我的套牌" << endl;
    cout << "5. 查看套牌详情" << endl;
    cout << "6. 导出套牌代码" << endl;
    cout << "7. 导入套牌代码" << endl;
    cout << "8. 退出" << endl;
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
            default:
                cout << "无效选择，请重试。" << endl;
        }
        
        // 清除输入缓冲区
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
    } while (choice != 8);
}
