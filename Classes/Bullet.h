#ifndef __BULLET_H__
#define __BULLET_H__

#include "BaseObject.h"
#include "Global.h"

#define BULLET_UP      1	  // BULLET����״̬
#define BULLET_DOWN    2      // BULLET����״̬
#define BULLET_LEFT    3      // BULLET����״̬
#define BULLET_RIGHT   4      // BULLET����״̬
enum BulletType
{
	NORMAL,
	PENETRATE,
};
class Bullet : public BaseObject
{
public:
	Bullet();
	~Bullet();
	static Bullet* create(Vec2 position, float speed, int dir);

	// ��ըʱ���Զ�����lifeΪ0
	void Blast();    

	Rect getRect() { return m_rect; };
	BulletType type = NORMAL;
	int hit_count = 0;
private:
	virtual bool init(Vec2 position, float speed, int dir,BulletType type=NORMAL);
	void update(float delta);
	void deleteObj(Sprite* obj);

private:
	Sprite*     m_sprite;       // ͼƬ����
	Texture2D*  m_texture;      // ����
	Rect        m_rect;			// ��ײ����
};

#endif
