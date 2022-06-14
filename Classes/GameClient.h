#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "cocos2d.h"
#include "Tank.h"
#include "Brick.h"
#include "aStar.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

#define OCEAN_ID 7
#define BRICK_ID 1
#define BLOCK_ID 3
#define FOREST_ID 5
#define NONE 0

#define MY_TANK_ID 110

USING_NS_CC;
using namespace cocos2d;

static int tankcount = 0;     // ��¼��ǰ̹����
static int NET_TAG = 11111;   

class GameClient : public Scene
{
public:
	GameClient();
	~GameClient();

	CREATE_FUNC(GameClient);
	virtual bool init();
	static Scene* createScene();
	void createBackGround();
	void update(float delta);
	void drawBigBG(Vec2 position);

	// �����紫������Ϣ������Ӧ
	void addTank(int id, float x, float y, int dir, int kind);
	void addFire(Tank* tank);

	// ʵ�ּ��̻ص�
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event); 

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };
	//����·���Ļ�ͼ�ڵ����
	DrawNode* m_draw;
private:
	Vector<Brick*>  m_bgList;     // �������б�
	Vector<Tank*>   m_tankList;   // ̹���б�
	Tank*           m_tank;       // ��̹��
	Vector<Tank*>	m_drawList;   // �ѻ��Ƶ�̹��
	
	Tank*           m_maxTank[50];        // �������ӿͻ���
	Vector<Tank*>   m_shouldFireList;     // ��¼��Ҫ�����̹�� - ������յ�������Ϣ��̹��

	Vector<Bullet*> m_deleteBulletList;   // ɾ���ӵ��б�
	Vector<Brick*>  m_deleteBrickList;    // ɾ��ש���б�
	Vector<Tank*>   m_deleteTankList;     // ɾ��̹���б�
	Size tileSize,visibleSize;
	TMXLayer* map_layer;
	int set_convey=0,can_convey = 0;
	Vec2 convey_p;
	void onMouseUp(Event *event) {
		EventMouse* e = (EventMouse*)event;
		convey_p.x =e->getCursorX();
		convey_p.y = e->getCursorY();
		set_convey = 1;
	}

	int enermy_num = 20;
	int max_num = 1;

	mapNode **m_map;
	void initMap() {
		//���ݵ�ͼ���߷�������ռ�
		m_map = new mapNode*[MAP_WIDTH];
		for (int n = 0; n < MAP_WIDTH; n++)
			m_map[n] = new mapNode[MAP_HEIGHT];
		vector<int> not_access_gid = {BRICK_ID,BLOCK_ID,OCEAN_ID};
		//����ɨ���ͼ����ÿһ����Ԫ
		for (int i = 0; i < MAP_WIDTH; i++)
		{
			for (int j = 0; j < MAP_HEIGHT; j++)
			{
				//����ǰλ��Ϊǽ����Ƭ����Ϊ����ͨ��
				bool flag = true;
				for (int id : not_access_gid) {
					if (map_layer->getTileGIDAt(Vec2(i, j)) == id) {
						flag = false;
						break;
					}
				}
				if (!flag)
				{
					mapNode temp = { NOT_ACCESS, i, j, 0, 0, 0, nullptr };
					m_map[i][j] = temp;
				}

				//��������Ϊ����ͨ��
				else
				{
					mapNode temp = { ACCESS, i, j, 0, 0, 0, nullptr };
					m_map[i][j] = temp;
				}
			}
		}
	}
	int aStar(mapNode** map, mapNode* origin, mapNode* destination,Tank *nowTank,float dt)
	{
		if (origin == destination) {
			return 0;
		}
		openList* open = new openList;
		open->next = nullptr;
		open->openNode = origin;
		closedList* close = new closedList;
		close->next = nullptr;
		close->closedNode = nullptr;
		//ѭ������8����������ڽڵ�
		while (checkNeighboringNodes(map, open, open->openNode, destination))
		{
			//��OPEN����ѡȡ�ڵ����CLOSED��
			insertNodeToClosedList(close, open);
			//��OPEN��Ϊ�գ�����Ѱ·ʧ��
			if (open == nullptr)
			{
				break;
			}
			//���յ���OPEN���У�����Ѱ·�ɹ�
			if (open->openNode->status == DESTINATION)
			{
				mapNode* tempNode = open->openNode;
				//����moveOnPath�����������ƾ�����·�����ƶ�
				moveOnPath(tempNode,nowTank,dt);
				break;
			}
		}
		return 0;
	}
	void moveOnPath(mapNode* tempNode,Tank *nowTank,float dt)
	{
		//�����洢·������Ľṹ��
		struct pathCoordinate { int x; int y; };
		//����·������ṹ������
		pathCoordinate* path = new pathCoordinate[MAP_WIDTH*MAP_HEIGHT];
		//���ø��ڵ���Ϣ����洢·������
		int loopNum = 0;
		while (tempNode != nullptr)
		{
			path[loopNum].x = tempNode->xCoordinate;
			path[loopNum].y = tempNode->yCoordinate;
			loopNum++;
			tempNode = tempNode->parent;
		}
		if (loopNum>=2) {
			int j = loopNum - 1;
			int fromX = path[j].x;
			int fromY = path[j].y;
			j--;
			int realX = path[j].x;
			int realY = path[j].y;
			if (realX - fromX == 0) {
				if (realY - fromY>0) {
					nowTank->MoveUP();
				}
				else if(realY - fromY < 0){
					nowTank->MoveDown();
				}
			}
			else if (realY-fromY==0) {
				if (realX - fromX > 0) {
					nowTank->MoveRight();
				}
				else if (realX - fromX < 0) {
					nowTank->MoveLeft();
				}
			}
		}
	}
	void updatePath(float dt) {
		auto nowTank = m_tank;
		int x = nowTank->getPositionX() / tileSize.width;
		int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
		int tmp = m_map[x][y].status;
		m_map[x][y].status = DESTINATION;
		auto m_dest = &m_map[x][y];
		for (int i = 0; i < m_tankList.size(); i++) {
			auto nowTank = m_tankList.at(i);
			if (nowTank->tank_kind == 1) {
				int x = nowTank->getPositionX() / tileSize.width;
				int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
				int tmp = m_map[x][y].status;
				m_map[x][y].status = ORIGIN;
				m_map[x][y].parent = nullptr;
				auto m_ori = &m_map[x][y];
				aStar(m_map, m_ori, m_dest,nowTank,dt);
				m_map[x][y].status = tmp;
			}
		}
		m_map[x][y].status = tmp;
	}
	void enermy_shoot(float dt) {
		auto nowTank = m_tank;
		int x_t = nowTank->getPositionX() / tileSize.width;
		int y_t = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
		for (int i = 0; i < m_tankList.size(); i++) {
			auto nowTank = m_tankList.at(i);
			if (nowTank->tank_kind == 1) {
				int x = nowTank->getPositionX() / tileSize.width;
				int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
			}
		}
	}
};

#endif