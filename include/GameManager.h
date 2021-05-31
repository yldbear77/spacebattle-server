#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

class GameManager {
public:
	static GameManager* GetInstance() {
		if (mInstance == nullptr) mInstance = new GameManager();
		return mInstance;
	}

private:
	static GameManager* mInstance;
	GameManager() {};
};

#endif