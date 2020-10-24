#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <time.h>
#include <sstream>
#include <algorithm>

// Версия игры.
const std::string version = "v1.0.0";

// В игре принимают участие два игрока: вы и Botbder. Для них отдельные классы.
class Player
{
public:
	Player(bool isBotbder);

	std::string getName() const;
	bool isBotbder() const;
	unsigned int getHealth() const;
	bool isDead() const;
	unsigned int getExtraMovesCount() const;

	void setName(std::string name);
	void setHealth(unsigned int hp);
	void damage(unsigned int hp);
	void heal(unsigned int hp);
	void addExtraMoves(unsigned int moves);
	void generateIDConflict();

	void retakeCards();
	void addNewCard(unsigned int id);
	void useCard(Player& enemy, unsigned int id);
	void move(Player& enemy, unsigned int ind);

	unsigned int getCardID(unsigned int ind) const;
	unsigned int getCardCount() const;
	void removeAllCards();
private:
	std::string _name;
	bool _isBotbder;
	unsigned int _health, _extraMoves;
	std::vector<unsigned int> _cardIDs;
	unsigned int _prevCardID;
};

typedef void (*MoveFunc)(Player&, Player&);

enum class CardType { Common, Epic, Player, Botbder };

// Класс карт, которыми будем ходить.
class Card
{
public:
	Card();
	Card(CardType type, std::string name, std::string description, MoveFunc move, MoveFunc nextMove);

	CardType getType() const;
	std::string getName() const;
	std::string getDescription() const;

	void move(Player& player, Player& enemy) const;
	void nextMove(Player& player, Player& enemy) const;
private:
	CardType _type;
	std::string _name, _description;
	MoveFunc _move, _nextMove;
};

// Менеджер карт. Хранит в себе все карты Великой битвы.
class CardManager
{
public:
	static void initCards();
	static Card getCardByID(unsigned int id);
	static unsigned int getAllCardsCount();
	static unsigned int getNewID(bool isBotbder);
	static unsigned int getRandomID(bool isBotbder);
	static void restoreEpicCards();
private:
	static std::vector<Card> _availableCards;
	static std::vector<unsigned int> _epicCardIDs, _forPlayerCardIDs, _forBotbderCardIDs;

	static void addNewCard(CardType type, std::string name, std::string description, MoveFunc move, MoveFunc nextMove);

	static void initEpicCards();
	static void initCommonCards();
	static void initPlayerCards();
	static void initBotbderCards();
};

enum class ConsoleColor
{
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Magenta = 5,
	Brown = 6,
	LightGray = 7,
	DarkGray = 8,
	LightBlue = 9,
	LightGreen = 10,
	LightCyan = 11,
	LightRed = 12,
	LightMagenta = 13,
	Yellow = 14,
	White = 15
};

// Вспомогательный класс для работы с консолью. А именно - цвета и русская локализация.
class Console
{
	static HANDLE _hOut;
public:
	static void setRusLocale();
	static void setConsoleColor(ConsoleColor text, ConsoleColor background = ConsoleColor::Black);
};

// Интерпретатор команд (таких как "!битва инфо").
class Command
{
public:
	Command(std::string input);

	std::string getCommand() const;
	std::string getArg(unsigned int ind) const;
	unsigned int getArgCount() const;
	bool isUnsignedNumber(unsigned int ind) const;
private:
	std::string _cmd;
	std::vector<std::string> _args;
};

// Основной класс всея игры.
class GreatBattle
{
public:
	GreatBattle();
	void run();

	void reset();
	void setNicknames();

	void showGreeting() const;
	void showCommands() const;

	void showRules() const;
	void showAllCards() const;
	void showInfo() const;
	void showAdversary() const;

	void showCard(unsigned int i, unsigned int id) const;
	
	bool moveStep(unsigned int i);

	void playerMove(unsigned int ind);
	void botbderMove(unsigned int ind);
	bool checkDead() const;
private:
	Player _you, _botbder;
};

int main()
{
	srand(time(nullptr));
	Console::setRusLocale();
	CardManager::initCards();
	GreatBattle gb;
	gb.run();
}

// ------------< GreatBattle >------------

GreatBattle::GreatBattle(): _you(false), _botbder(true)
{
	showGreeting();
	setNicknames();
	reset();
}

void GreatBattle::run()
{
	Console::setConsoleColor(ConsoleColor::LightMagenta);
	std::cout << "Великая битва началась!" << std::endl;
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << "Введите !помощь для вывода списка команд. " << std::endl;
	std::string input;

	bool retaked = false;
	while (true)
	{
		Console::setConsoleColor(ConsoleColor::White);
		std::cout << "> ";
		std::getline(std::cin, input);
		Command command(input);
		if (command.getCommand() == "!помощь")
			showCommands();
		else if (command.getCommand() == "!выход")
			break;
		else if (command.getCommand() == "!битва")
		{
			if (command.getArgCount() == 0)
				showRules();
			else if (command.getArg(0) == "карты")
				showAllCards();
			else if (command.getArg(0) == "инфо")
				showInfo();
			else if (command.getArg(0) == "противник")
				showAdversary();
			else if (command.getArg(0) == "пересдать")
			{
				if (retaked)
				{
					Console::setConsoleColor(ConsoleColor::Red);
					std::cout << "Вы более не можете пересдать карты!" << std::endl;
				}
				else
				{
					Console::setConsoleColor(ConsoleColor::LightGreen);
					std::cout << "Карты пересданы." << std::endl;
					_you.retakeCards();
					retaked = true;
				}
			}
			else if (command.isUnsignedNumber(0))
			{
				unsigned int ind = std::stoul(command.getArg(0));
				if (ind > _you.getCardCount() || ind == 0)
				{
					Console::setConsoleColor(ConsoleColor::Red);
					std::cout << "У вас нет такой карты!" << std::endl;
					continue;
				}

				if (!moveStep(ind))
				{
					reset();
					retaked = false;
				}
				else
					retaked = true;
			}
		}
		else
		{
			Console::setConsoleColor(ConsoleColor::Red);
			std::cout << "Команда не найдена!" << std::endl;
		}
	}
}


void GreatBattle::reset()
{
	_you.setHealth(5);
	_botbder.setHealth(5);

	_you.removeAllCards();
	_botbder.removeAllCards();

	CardManager::restoreEpicCards();

	for (int i = 0; i < 3; i++)
	{
		_you.addNewCard(CardManager::getNewID(false));
		_botbder.addNewCard(CardManager::getNewID(true));
	}
}

void GreatBattle::setNicknames()
{
	Console::setConsoleColor(ConsoleColor::White);
	std::cout << "Введите свой никнейм: " << std::endl;
	std::cout << "> ";
	std::string nick;
	std::getline(std::cin, nick);
	_you.setName(nick);
	_botbder.setName("Botbder");
}


void GreatBattle::showGreeting() const
{
	Console::setConsoleColor(ConsoleColor::LightMagenta);
	std::cout << "Добро пожаловать на Великую битву!" << std::endl;
	std::cout << "Великая битва - это коллекционная карточная игра в консольном режиме по Всемирью, нашей фэнтези-вселенной." << std::endl;
	std::cout << "Версия игры: ";
	Console::setConsoleColor(ConsoleColor::LightRed);
	std::cout << version << std::endl;
	std::cout << "Автор идеи: Mrakovey" << std::endl;
	std::cout << "Реализатор: DmitryWS" << std::endl;
	Console::setConsoleColor(ConsoleColor::LightMagenta);
	std::cout << "Удачи, боец, и да хранит тебя Аркана!" << std::endl;
}

void GreatBattle::showCommands() const
{
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << "Общие команды:" << std::endl;
	std::cout << "!помощь - общий список команд;" << std::endl;
	std::cout << "!выход - выход из игры." << std::endl;
	std::cout << "!битва - правила Великой битвы." << std::endl;
}


void GreatBattle::showRules() const
{
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << "В начале битвы вам и вашему противнику Botbder'у выдаются три карты." << std::endl;
	std::cout << "У вас изначально 5 жизней, как и у Botbder. Ходы делаются поочерёдно, начиная с вас." << std::endl;
	std::cout << "За ход можно использовать не более одной карты. Каждый ход вы вытягиваете ещё одну карту." << std::endl;
	std::cout << "Проигрывает тот, у кого заканчиваются жизни." << std::endl;
	std::cout << "!битва карты - информация о картах Великой битвы;" << std::endl;
	std::cout << "!битва инфо - информация о ваших жизнях и картах;" << std::endl;
	std::cout << "!битва противник - информация о жизнях Botbder;" << std::endl;
	std::cout << "!битва [номер карты] - сыграть нужную карту;" << std::endl;
	std::cout << "!битва пересдать - пересдать себе карты на первом ходу (один раз за битву)." << std::endl;
}

void GreatBattle::showAllCards() const
{
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << "Цветовые обозначения:" << std::endl;
	
	Console::setConsoleColor(ConsoleColor::LightMagenta);
	std::cout << "###";
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << " - данные карты существуют в единственном экземпляре и могут применяться один раз за битву." << std::endl;
	Console::setConsoleColor(ConsoleColor::Yellow);
	std::cout << "###";
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << " - данные карты могут повторяться при выдаче." << std::endl;
	Console::setConsoleColor(ConsoleColor::LightCyan);
	std::cout << "###";
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << " - данные карты доступны только игроку." << std::endl;
	Console::setConsoleColor(ConsoleColor::Cyan);
	std::cout << "###";
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << " - данные карты доступны только Botbder'у." << std::endl;

	for (int i = 1; i <= CardManager::getAllCardsCount(); i++)
		showCard(i, i);
}

void GreatBattle::showInfo() const
{
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << "Ваше здоровье: " << _you.getHealth() << " ед." << std::endl;
	std::cout << "Ваши карты: " << std::endl;
	for (int i = 0; i < _you.getCardCount(); i++)
		showCard(i + 1, _you.getCardID(i));
}

void GreatBattle::showAdversary() const
{
	Console::setConsoleColor(ConsoleColor::LightGreen);
	std::cout << "Здоровье Botbder'а: " << _botbder.getHealth() << " ед." << std::endl;
}


void GreatBattle::showCard(unsigned int i, unsigned int id) const
{
	Card card = CardManager::getCardByID(id);
	switch (card.getType())
	{
	case CardType::Epic:
		Console::setConsoleColor(ConsoleColor::LightMagenta);
		break;
	case CardType::Common:
		Console::setConsoleColor(ConsoleColor::Yellow);
		break;
	case CardType::Player:
		Console::setConsoleColor(ConsoleColor::LightCyan);
		break;
	case CardType::Botbder:
		Console::setConsoleColor(ConsoleColor::Cyan);
		break;
	}
	std::cout << i << ") " << card.getName() << " - " << card.getDescription() << std::endl;
}


bool GreatBattle::moveStep(unsigned int i)
{
	playerMove(i);
	if (checkDead())
		return false;
	_you.addNewCard(CardManager::getNewID(false));
	if (_you.getExtraMovesCount() > 0)
		return true;

	do
	{
		botbderMove(rand() % _botbder.getCardCount() + 1);
		if (checkDead())
		{
			return false;
		}
		_botbder.addNewCard(CardManager::getNewID(true));
	} while (_botbder.getExtraMovesCount() > 0);

	return true;
}

void GreatBattle::playerMove(unsigned int ind)
{
	Card card = CardManager::getCardByID(_you.getCardID(ind - 1));
	Console::setConsoleColor(ConsoleColor::LightBlue);
	std::cout << "Вы использовали карту \"" << card.getName() << "\"!" << std::endl;
	_you.move(_botbder, ind - 1);
}

void GreatBattle::botbderMove(unsigned int ind)
{
	Card card = CardManager::getCardByID(_botbder.getCardID(ind - 1));
	Console::setConsoleColor(ConsoleColor::LightBlue);
	std::cout << "Botbder использовал карту \"" << card.getName() << "\"!" << std::endl;
	_botbder.move(_you, ind - 1);
}

bool GreatBattle::checkDead() const
{
	Console::setConsoleColor(ConsoleColor::Blue);
	if (_you.isDead() && !_botbder.isDead())
	{
		std::cout << "Упс... Вы проиграли, " << _you.getName() << ", хе-хе!" << std::endl;
		return true;
	}
	if (!_you.isDead() && _botbder.isDead())
	{
		std::cout << "Е-ей! Вы выиграли, " << _you.getName() << "! :D Восславим же Аркану!" << std::endl;
		return true;
	}
	if (_you.isDead() && _botbder.isDead())
	{
		std::cout << "Аммок меня побери, вы оба проиграли!? Ну ничёси..." << std::endl;
		return true;
	}
	return false;
}

// ------------< CardManager >------------

std::vector<Card> CardManager::_availableCards;
std::vector<unsigned int> CardManager::_epicCardIDs, CardManager::_forPlayerCardIDs, CardManager::_forBotbderCardIDs;

void CardManager::initCards()
{
	initEpicCards();
	initCommonCards();
	initPlayerCards();
	initBotbderCards();
}

Card CardManager::getCardByID(unsigned int id)
{
	if (1 <= id && id <= _availableCards.size())
		return _availableCards[id - 1];
	return Card();
}

unsigned int CardManager::getAllCardsCount()
{
	return _availableCards.size();
}

unsigned int CardManager::getNewID(bool isBotbder)
{
	unsigned int ind, id;
	if (isBotbder)
	{
		ind = rand() % _forBotbderCardIDs.size();
		id = _forBotbderCardIDs[ind];
		if (CardManager::getCardByID(id).getType() == CardType::Epic)
		{
			ind = rand() % _forBotbderCardIDs.size();
			id = _forBotbderCardIDs[ind];
			if (CardManager::getCardByID(id).getType() == CardType::Epic)
				_forBotbderCardIDs.erase(_forBotbderCardIDs.begin() + ind);
		}
	}
	else
	{
		ind = rand() % _forPlayerCardIDs.size();
		id = _forPlayerCardIDs[ind];
		if (CardManager::getCardByID(id).getType() == CardType::Epic)
		{
			ind = rand() % _forPlayerCardIDs.size();
			id = _forPlayerCardIDs[ind];
			if (CardManager::getCardByID(id).getType() == CardType::Epic)
				_forPlayerCardIDs.erase(_forPlayerCardIDs.begin() + ind);
		}
	}
	return id;
}

unsigned int CardManager::getRandomID(bool isBotbder)
{
	if (isBotbder)
		return _forBotbderCardIDs[rand() % _forBotbderCardIDs.size()];
	else
		return _forPlayerCardIDs[rand() % _forPlayerCardIDs.size()];
}

void CardManager::restoreEpicCards()
{
	for (int i = 0; i < _availableCards.size(); i++)
		if (_availableCards[i].getType() == CardType::Epic)
		{
			if (std::find(_forBotbderCardIDs.begin(), _forBotbderCardIDs.end(), i + 1) == _forBotbderCardIDs.end())
				_forBotbderCardIDs.push_back(i + 1);
			if (std::find(_forPlayerCardIDs.begin(), _forPlayerCardIDs.end(), i + 1) == _forPlayerCardIDs.end())
				_forPlayerCardIDs.push_back(i + 1);
		}
}


void CardManager::addNewCard(CardType type, std::string name, std::string description, MoveFunc move, MoveFunc nextMove)
{
	_availableCards.push_back(Card(type, name, description, move, nextMove));
	unsigned int id = _availableCards.size();
	switch (type)
	{
	case CardType::Epic:
		_epicCardIDs.push_back(id);
		_forPlayerCardIDs.push_back(id);
		_forBotbderCardIDs.push_back(id);
		break;
	case CardType::Common:
		_forPlayerCardIDs.push_back(id);
		_forBotbderCardIDs.push_back(id);
		break;
	case CardType::Player:
		_forPlayerCardIDs.push_back(id);
		break;
	case CardType::Botbder:
		_forBotbderCardIDs.push_back(id);
		break;
	}
}

void CardManager::initEpicCards()
{
	addNewCard(CardType::Epic, "Конструктор Парадоксов (Посох Мудреца)",
		"Наносит случайным образом 4 ед. урона вам, противнику или обоим сразу.",
		[](Player& p, Player& e)
		{
			int d3 = rand() % 3;
			switch (d3)
			{
			case 0:
				p.damage(4);
				break;
			case 1:
				e.damage(4);
				break;
			case 2:
				p.damage(4);
				e.damage(4);
				break;
			}
		},
		nullptr
		);
	addNewCard(CardType::Epic, "Курохай (Катана Изаму)",
		"Наносит противнику 1 ед. урона и еще 2 ед. урона на следующий ход.",
		[](Player& p, Player& e)
		{
			e.damage(1);
		},
		[](Player& p, Player& e)
		{
			e.damage(2);
		}
	);
	addNewCard(CardType::Epic, "Меч Земли (Клинок Трискелиона)",
		"Наносит 2 ед. урона и добавляет вам 1 жизнь.",
		[](Player& p, Player& e)
		{
			e.damage(2);
			p.heal(1);
		},
		nullptr
	);
	addNewCard(CardType::Epic, "Меч Небес (Клинок Трискелиона)",
		"Наносит случайным образом 1, 2 или 3 ед. урона.",
		[](Player& p, Player& e)
		{
			e.damage(rand() % 3 + 1);
		},
		nullptr
	);
	addNewCard(CardType::Epic, "Меч Морей (Клинок Трискелиона)",
		"Наносит 2 ед. урона, противник пропускает ход.",
		[](Player& p, Player& e)
		{
			e.damage(2);
			p.addExtraMoves(1);
		},
		nullptr
	);
	addNewCard(CardType::Epic, "Алое Пламя (Перстень Зариака)",
		"В следующий ход нанесет 2 ед. урона противнику.",
		nullptr,
		[](Player& p, Player& e)
		{
			e.damage(2);
		}
	);
	addNewCard(CardType::Epic, "Доспех Чёрной Розы (Броня Лорда Рейвена)",
		"Добавляет вам 2 жизни.",
		[](Player& p, Player& e)
		{
			p.heal(2);
		},
		nullptr
	);
	addNewCard(CardType::Epic, "Яропламень (Меч Князя Велемира)",
		"Наносит 3 ед. урона.",
		[](Player& p, Player& e)
		{
			e.damage(3);
		},
		nullptr
	);
	 addNewCard(CardType::Epic, "Глаз Суккуба (Амулет Мизерис)",
		"Противник пропускает 2 хода.",
		[](Player& p, Player& e)
		{
			p.addExtraMoves(2);
		},
		nullptr
	);
}

void CardManager::initCommonCards()
{
	addNewCard(CardType::Common, "Алмазный меч", "Наносит 1 ед. урона.",
		[](Player& p, Player& e) { e.damage(1); }, nullptr);
	addNewCard(CardType::Common, "Рунический щит", "Добавляет 2 жизни.",
		[](Player& p, Player& e) { p.heal(2); }, nullptr);
	addNewCard(CardType::Common, "Костяной лук", "Наносит 1 или 2 ед. урона.",
		[](Player& p, Player& e) { e.damage(rand() % 2 + 1); }, nullptr);
	addNewCard(CardType::Common, "Зелье лечения", "Восстанавливает 1 жизнь.",
		[](Player& p, Player& e) { p.heal(1); }, nullptr);
	addNewCard(CardType::Common, "Зелье регенерации", "Восстанавливает 1 жизнь в следующий ваш ход.",
		nullptr, [](Player& p, Player& e) { p.heal(1); });
	addNewCard(CardType::Common, "TNT", "Наносит вам и противнику 3 ед. урона.",
		[](Player& p, Player& e) { p.damage(3); e.damage(3); }, nullptr);
	addNewCard(CardType::Common, "Деревянный топор", "С вероятность 30% наносит 1 ед. урона.",
		[](Player& p, Player& e) { if (rand() % 10 < 3) e.damage(1); }, nullptr);
	addNewCard(CardType::Common, "MRU-пушка", "Наносит 3 ед. урона.",
		[](Player& p, Player& e) { e.damage(3); }, nullptr);
	addNewCard(CardType::Common, "Бумеранг", "Наносит 1 ед. урона, предмет не теряется после использования.",
		[](Player& p, Player& e) { e.damage(1); p.addNewCard(18); }, nullptr);
	addNewCard(CardType::Common, "Алмазная броня", "Добавляет 1 жизнь.",
		[](Player& p, Player& e) { p.heal(1); }, nullptr);
	addNewCard(CardType::Common, "Ведро лавы", "Наносит вам и противнику 1 ед. урона.",
		[](Player& p, Player& e) { p.damage(1); e.damage(1); }, nullptr);
	addNewCard(CardType::Common, "Хитрый механизм", "Вы или противник пропускает ход, определяется случайным образом.",
		[](Player& p, Player& e) { if (rand() % 2 == 0) p.addExtraMoves(1); else e.addExtraMoves(1); }, nullptr);
	addNewCard(CardType::Common, "Блок булыжника", "Ничего не делает, просто занимает место и пропадает при использовании.",
		nullptr, nullptr);
	addNewCard(CardType::Common, "Ихориевый меч", "Наносит 4 ед. урона.",
		[](Player& p, Player& e) { e.damage(4); }, nullptr);
	addNewCard(CardType::Common, "Золотой меч", "Наносит 1 ед урона с вероятностью в 50%.",
		[](Player& p, Player& e) { if (rand() % 2 == 0 == 1) e.damage(1); }, nullptr);
	addNewCard(CardType::Common, "Посох тауматурга", "Наносит 2 ед. урона.",
		[](Player& p, Player& e) { e.damage(2); }, nullptr);
	addNewCard(CardType::Common, "Кровавый меч", "Наносит 1 ед урона и восстанавливает вам 1 жизнь.",
		[](Player& p, Player& e) { e.damage(1); p.heal(1); }, nullptr);
	addNewCard(CardType::Common, "Жертвенный кинжал", "Наносит вам 1 ед. урона.",
		[](Player& p, Player& e) { p.damage(1); }, nullptr);
	addNewCard(CardType::Common, "Снежок", "Противник пропускает ход, с 50% вероятности наносит 1 ед. урона.",
		[](Player& p, Player& e) { p.addExtraMoves(1); if (rand() % 2 == 0) e.damage(1); }, nullptr);
	addNewCard(CardType::Common, "Взрывное зелье исцеления", "Восстанавливает вам и противнику 1 ед. здоровья.",
		[](Player& p, Player& e) { p.heal(1); e.heal(1); }, nullptr);
	addNewCard(CardType::Common, "Верстак", "Вы получаете две карты.",
		[](Player& p, Player& e) { p.addNewCard(CardManager::getNewID(p.isBotbder()));
								   p.addNewCard(CardManager::getNewID(p.isBotbder())); }, nullptr);

	addNewCard(CardType::Common, "Взрывное зелье отравления", "Наносит 1 ед. урона вам и противнику в следующий ваш ход.",
		nullptr, [](Player& p, Player& e) { p.damage(1); e.damage(1); });
	addNewCard(CardType::Common, "Взрывное зелье урона", "Наносит вам и противнику 1 ед. урона.",
		[](Player& p, Player& e) { p.damage(1); e.damage(1); }, nullptr);
	addNewCard(CardType::Common, "Варочная стойка", "Даёт два случайных зелья.",
		[](Player& p, Player& e)
		{
			int potionIDs[5] = { 13, 14, 29, 31, 32 };
			p.addNewCard(potionIDs[rand() % 5]);
			p.addNewCard(potionIDs[rand() % 5]);
		}, nullptr);
}

void CardManager::initPlayerCards()
{
	addNewCard(CardType::Player, "Непонятная ерунда", "Активирует эффект случайного предмета.",
		[](Player& p, Player& e)
		{
			p.useCard(e, CardManager::getRandomID(false));
		}, nullptr);
}

void CardManager::initBotbderCards()
{
	addNewCard(CardType::Botbder, "Краш", "Игрок пропускает ход и получает 1 ед. урона.",
		[](Player& p, Player& e) { p.addExtraMoves(1); e.damage(1); }, nullptr);
	addNewCard(CardType::Botbder, "ID-конфликт", "Заменяет игроку один предмет на другой случайный.",
		[](Player& p, Player& e) { e.generateIDConflict(); }, nullptr);
	addNewCard(CardType::Botbder, "Дисконнект", "Игрок пропускает два хода.",
		[](Player& p, Player& e) { p.addExtraMoves(2); }, nullptr);
}

// ------------< Console >------------

HANDLE Console::_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

void Console::setRusLocale()
{
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);
}

void Console::setConsoleColor(ConsoleColor text, ConsoleColor background)
{
	SetConsoleTextAttribute(_hOut, (WORD)(((unsigned)background << 4) | (unsigned)text));
}

// ------------< Command >------------

Command::Command(std::string input)
{
	std::stringstream ss(input);
	std::getline(ss, _cmd, ' ');
	
	std::string arg;
	while (std::getline(ss, arg, ' '))
		_args.push_back(arg);
}


std::string Command::getCommand() const
{
	return _cmd;
}

std::string Command::getArg(unsigned int ind) const
{
	if (ind >= _args.size())
		return "";
	return _args[ind];
}

unsigned int Command::getArgCount() const
{
	return _args.size();
}

bool Command::isUnsignedNumber(unsigned int ind) const
{
	if (ind >= _args.size())
		return false;
	std::string s = _args[ind];
	for (char c: s)
		if (c < '0' || c > '9')
			return false;
	return true;
}

// ------------< Player >------------

Player::Player(bool isBotbder) : _name(""), _isBotbder(isBotbder), _health(5), _extraMoves(0), _prevCardID(0) {}


std::string Player::getName() const
{
	return _name;
}

bool Player::isBotbder() const
{
	return _isBotbder;
}

unsigned int Player::getHealth() const
{
	return _health;
}

bool Player::isDead() const
{
	return _health == 0;
}

unsigned int Player::getExtraMovesCount() const
{
	return _extraMoves;
}


void Player::setName(std::string name)
{
	_name = name;
}

void Player::setHealth(unsigned int hp)
{
	_health = hp;
}

void Player::damage(unsigned int hp)
{
	if (_health < hp)
		_health = 0;
	else
		_health -= hp;
	Console::setConsoleColor(ConsoleColor::LightRed);
	std::cout << "Игроку " << _name << " был нанесён урон в " << hp << " ед." << std::endl;
}

void Player::heal(unsigned int hp)
{
	_health += hp;
	Console::setConsoleColor(ConsoleColor::LightRed);
	std::cout << "Игрок " << _name << " исцелился на " << hp << " ед." << std::endl;
}

void Player::addExtraMoves(unsigned int moves)
{
	_extraMoves += moves;
	Console::setConsoleColor(ConsoleColor::LightRed);
	std::cout << "Игрок " << _name << " получил дополнительные " << moves << " ход(а)." << std::endl;
}

void Player::generateIDConflict()
{
	unsigned int ind = rand() % _cardIDs.size();
	_cardIDs[ind] = CardManager::getRandomID(_isBotbder);
}


void Player::retakeCards()
{
	_cardIDs.clear();
	for (int i = 0; i < 3; i++)
		addNewCard(CardManager::getNewID(_isBotbder));
}

void Player::addNewCard(unsigned int id)
{
	_cardIDs.push_back(id);
}

void Player::useCard(Player& enemy, unsigned int id)
{
	CardManager::getCardByID(id).move(*this, enemy);
	CardManager::getCardByID(_prevCardID).nextMove(*this, enemy);
	_prevCardID = id;
}

void Player::move(Player& enemy, unsigned int ind)
{
	if (_extraMoves > 0)
		_extraMoves--;
	useCard(enemy, _cardIDs[ind]);
	_cardIDs.erase(_cardIDs.begin() + ind);
}


unsigned int Player::getCardID(unsigned int ind) const
{
	if (ind < _cardIDs.size())
		return _cardIDs[ind];
	return 0;
}

unsigned int Player::getCardCount() const
{
	return _cardIDs.size();
}

void Player::removeAllCards()
{
	_cardIDs.clear();
	_prevCardID = 0;
}

// ------------< Card >------------

Card::Card() : _type(CardType::Common), _name("???"), _description("Вы забудете о моём существовании."),
_move(nullptr), _nextMove(nullptr) {}

Card::Card(CardType type, std::string name, std::string description, MoveFunc move, MoveFunc nextMove) :
	_type(type), _name(name), _description(description), _move(move), _nextMove(nextMove) {}


CardType Card::getType() const
{
	return _type;
}

std::string Card::getName() const
{
	return _name;
}

std::string Card::getDescription() const
{
	return _description;
}


void Card::move(Player& player, Player& enemy) const
{
	if (_move)
		_move(player, enemy);
}

void Card::nextMove(Player& player, Player& enemy) const
{
	if (_nextMove)
		_nextMove(player, enemy);
}
