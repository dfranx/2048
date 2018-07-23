#include "Game.h"
#include <deque>
#include <unordered_map>
#include <SFML/Graphics/RectangleShape.hpp>

namespace tw
{
	Game::Game(int w, int h)
	{
		tileSize = (w - FIELD_MARGIN * 2 - TILE_MARGIN * (FIELD_WIDTH - 1)) / FIELD_WIDTH;
		animState = false;

		if (!font.loadFromFile("ClearSans.ttf"))
			printf("Failed to load ClearSans.ttf\n");


		Reset();
	}

	void Game::OnEvent(sf::Event e)
	{
		if (e.type == sf::Event::KeyPressed) {
			if (animState)
				return;

			sf::Keyboard::Key kc = e.key.code;

			if (kc == sf::Keyboard::Left || kc == sf::Keyboard::A)
				move(-1, 0);
			else if (kc == sf::Keyboard::Right || kc == sf::Keyboard::D)
				move(1, 0);
			else if (kc == sf::Keyboard::Up || kc == sf::Keyboard::W)
				move(0, -1);
			else if (kc == sf::Keyboard::Down || kc == sf::Keyboard::S)
				move(0, 1);
		}
	}

	void Game::Update()
	{
		if (animState) {
			if (animClock.getElapsedTime().asSeconds() >= ANIMATION_DURATION) {
				animState = false;

				for (int i = 0; i < moves.size(); i++) {
					sf::Vector2i f = moves[i].first.first;
					sf::Vector2i t = moves[i].first.second;

					char srcVal = moves[i].second;
					char destVal = map[t.x][t.y];

					map[f.x][f.y] = 0;

					if (srcVal == destVal && f != t)
						map[t.x][t.y] = srcVal + 1;
					else
						map[t.x][t.y] = srcVal;
				}

				moves.clear();

				Spawn();

				return;
			}


		}
	}

	void Game::Render(sf::RenderTarget& tgt)
	{
		sf::Text text;
		text.setFont(font);
		text.setStyle(sf::Text::Style::Bold);

		sf::RectangleShape tile;
		tile.setSize(sf::Vector2f(tileSize, tileSize));
		
		for (int x = 0; x < FIELD_WIDTH; x++) {
			for (int y = 0; y < FIELD_HEIGHT; y++) {
				tile.setPosition(FIELD_MARGIN + x * (tileSize + TILE_MARGIN), FIELD_MARGIN + y * (tileSize + TILE_MARGIN));
				tile.setFillColor(getTileColor(map[x][y]));
				
				tgt.draw(tile);

				if (map[x][y] != 0) {
					text.setCharacterSize(getTextSize(map[x][y]));
					text.setString(getText(map[x][y]));

					sf::FloatRect fr = text.getLocalBounds();

					text.setPosition(FIELD_MARGIN + x * (tileSize + TILE_MARGIN) + (tileSize - fr.width) / 2 - fr.left, FIELD_MARGIN + y * (tileSize + TILE_MARGIN) + (tileSize - fr.height) / 2 - fr.top);
					text.setFillColor(getTextColor(map[x][y]));

					tgt.draw(text);
				}
			}
		}

		sf::Vector2i fMargin(FIELD_MARGIN, FIELD_MARGIN);

		for (int i = 0; i < moves.size(); i++) {
			sf::Vector2i orig = moves[i].first.first;
			sf::Vector2i f = fMargin + orig * (tileSize + TILE_MARGIN);
			sf::Vector2i t = fMargin + moves[i].first.second * (tileSize + TILE_MARGIN);
			
			float movePercent = (animClock.getElapsedTime().asSeconds() / ANIMATION_DURATION);
			sf::Vector2f curPos = sf::Vector2f(f) + sf::Vector2f(t - f) * movePercent;

			text.setCharacterSize(getTextSize(moves[i].second));
			text.setString(getText(moves[i].second));

			sf::FloatRect fr = text.getLocalBounds();

			text.setPosition(curPos.x + (tileSize - fr.width) / 2 - fr.left, curPos.y + (tileSize - fr.height) / 2 - fr.top);
			text.setFillColor(getTextColor(moves[i].second));

			tile.setPosition(curPos);
			tile.setFillColor(getTileColor(moves[i].second));

			tgt.draw(tile);
			tgt.draw(text);
		}
	}

	void Game::Spawn()
	{
		int availableCount = 0;
		sf::Vector2i availableMoves[FIELD_WIDTH*FIELD_HEIGHT];

		for (int x = 0; x < FIELD_WIDTH; x++) {
			for (int y = 0; y < FIELD_HEIGHT; y++) {
				if (map[x][y] == 0) {
					availableMoves[availableCount] = sf::Vector2i(x, y);
					availableCount++;
				}
			}
		}

		if (availableCount == 0) {
			Reset();
			return;
		}

		sf::Vector2i newPos = availableMoves[rand() % availableCount];
		char newTileID = (rand() % 10) < 9 ? 1 : 2;

		map[newPos.x][newPos.y] = newTileID;		
	}

	void Game::Reset()
	{
		for (int x = 0; x < FIELD_WIDTH; x++)
			for (int y = 0; y < FIELD_HEIGHT; y++)
				map[x][y] = 0;
		Spawn();
	}

	sf::Color Game::getTileColor(char tile)
	{
		static const sf::Color colors[] = {
			sf::Color(238, 228, 218, 97),		// empty
			sf::Color(238, 228, 218),			// 2^1 == 2
			sf::Color(237, 224, 200),			// 2^2 == 4
			sf::Color(242, 177, 121),			// 2^3 == 8
			sf::Color(245, 149, 99),			// 2^4 == 16
			sf::Color(246, 124, 95),			// 2^5 == 32
			sf::Color(246, 94, 59),				// 2^6 == 64
			sf::Color(237, 207, 114),			// 2^7 == 128
			sf::Color(237, 204, 97),			// 2^8 == 256
			sf::Color(237, 200, 80),			// 2^9 == 512
			sf::Color(237, 197, 63),			// 2^10 == 1024
			sf::Color(237, 194, 46),			// 2^11 == 2048
		};
		return colors[tile];
	}

	sf::Color Game::getTextColor(char tile)
	{
		if (tile >= 3) // tile >= 8 (cuz 2^3 == 8)
			return sf::Color(249, 246, 242);
		return sf::Color(119, 110, 101);
	}

	std::string Game::getText(char tile)
	{
		// this is small optimization - we dont have to use pow()
		static const std::string text[] = {
			"",		// empty
			"2",			// 2^1 == 2
			"4",			// 2^2 == 4
			"8",			// 2^3 == 8
			"16",			// 2^4 == 16
			"32",			// 2^5 == 32
			"64",			// 2^6 == 64
			"128",			// 2^7 == 128
			"256",			// 2^8 == 256
			"512",			// 2^9 == 512
			"1024",			// 2^10 == 1024
			"2048"			// 2^11 == 2048
		};
		return text[tile];
	}

	int Game::getTextSize(char tile)
	{
		if (tile >= 10)
			return 35;
		else if (tile >= 7)
			return 45;
		return 55;
	}

	void Game::move(char dirX, char dirY)
	{
		// copy map
		for (int x = 0; x < FIELD_WIDTH; x++)
			for (int y = 0; y < FIELD_HEIGHT; y++)
				tempMap[x][y] = map[x][y];

		if (dirX == -1) {
			for (int x = 1; x < FIELD_WIDTH; x++) {
				for (int y = 0; y < FIELD_HEIGHT; y++) {
					if (tempMap[x][y] == 0)
						continue;

					sf::Vector2i finalPos = sf::Vector2i(x, y);

					for (int mx = x - 1; mx >= 0; mx--) {
						finalPos = sf::Vector2i(mx, y);

						if (tempMap[mx][y] != 0)
							break;
					}

					applyMove(sf::Vector2i(x, y), finalPos, dirX, dirY);
				}
			}
		}
		else if (dirX == 1) {
			for (int x = FIELD_WIDTH-2; x >= 0; x--) {
				for (int y = 0; y < FIELD_HEIGHT; y++) {
					if (tempMap[x][y] == 0)
						continue;

					sf::Vector2i finalPos = sf::Vector2i(x, y);

					for (int mx = x + 1; mx < FIELD_WIDTH; mx++) {
						finalPos = sf::Vector2i(mx, y);

						if (tempMap[mx][y] != 0)
							break;
					}

					applyMove(sf::Vector2i(x, y), finalPos, dirX, dirY);
				}
			}
		}
		else if (dirY == -1) {
			for (int y = 1; y < FIELD_HEIGHT; y++) {
				for (int x = 0; x < FIELD_WIDTH; x++) {
					if (tempMap[x][y] == 0)
						continue;

					sf::Vector2i finalPos = sf::Vector2i(x, y);

					for (int my = y - 1; my >= 0; my--) {
						finalPos = sf::Vector2i(x, my);

						if (tempMap[x][my] != 0)
							break;
					}

					applyMove(sf::Vector2i(x, y), finalPos, dirX, dirY);
				}
			}
		}
		else if (dirY == 1) {
			for (int y = FIELD_HEIGHT-2; y >= 0; y--) {
				for (int x = 0; x < FIELD_WIDTH; x++) {
					if (tempMap[x][y] == 0)
						continue;

					sf::Vector2i finalPos = sf::Vector2i(x, y);

					for (int my = y + 1; my < FIELD_HEIGHT; my++) {
						finalPos = sf::Vector2i(x, my);

						if (tempMap[x][my] != 0)
							break;
					}

					applyMove(sf::Vector2i(x, y), finalPos, dirX, dirY);
				}
			}
		}

		bool isFilled = true;
		bool isGameOver = true;
		for (int x = 0; x < FIELD_WIDTH; x++) {
			for (int y = 0; y < FIELD_HEIGHT; y++) {
				if (tempMap[x][y] == 0) {
					isFilled = false;
					isGameOver = false;
					break;
				}

				char val = tempMap[x][y];

				if (x != 0 && tempMap[x - 1][y] == val)
					isGameOver = false;
				else if (y != 0 && tempMap[x][y - 1] == val)
					isGameOver = false;
				else if (x != FIELD_WIDTH - 1 && tempMap[x + 1][y] == val)
					isGameOver = false;
				else if (y != FIELD_HEIGHT - 1 && tempMap[x][y+1] == val)
					isGameOver = false;
			}

			if (!isFilled || !isGameOver)
				break;
		}

		if (isGameOver)
			Reset();
	}

	void Game::applyMove(sf::Vector2i f, sf::Vector2i t, int dx, int dy)
	{
		char srcVal = tempMap[f.x][f.y];
		char destVal = tempMap[t.x][t.y];

		tempMap[f.x][f.y] = 0;

		if (destVal == srcVal) {
			tempMap[t.x][t.y] = srcVal + 1;
			if (srcVal + 1 == 12) Reset();
		} else
			tempMap[t.x - (dx * (destVal != 0))][ t.y - (dy * (destVal != 0))] = srcVal;

		sf::Vector2i from = f, to;

		if (destVal == srcVal)
			to = t;
		else
			to = sf::Vector2i(t.x - (dx * (destVal != 0)), t.y - (dy * (destVal != 0)));

		if (from != to) {
			map[f.x][f.y] = 0;

			moves.push_back(std::make_pair(std::make_pair(from, to), srcVal));

			animState = true;
			animClock.restart();
		}
	}
}