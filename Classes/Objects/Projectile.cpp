#include "Projectile.h"
#include "Engine.h"
#include "Objects\LevelGrid.h"
#include "Objects\Bonus.h"
#include "Scenes\GameplayScene.h"

USING_NS_CC;

#define PROJ_DELAY 0.04f

Projectile::Projectile()
	: GameObject(Type::PROJECTILE)
{
}

Projectile::~Projectile()
{
}

bool Projectile::init()
{
	GameObject::init();

	_sprite = Sprite::createWithSpriteFrameName("projectile.png");
	this->addChild(_sprite);

	auto moving = Sequence::create(CallFunc::create(CC_CALLBACK_0(Projectile::onMoved, this))
		, DelayTime::create(PROJ_DELAY), nullptr);
	_sprite->runAction(RepeatForever::create(moving));

	return true;
}

void Projectile::move(Direction direction)
{
	// adjust position
	if (direction == GameObject::Direction::LEFT)
		_sprite->setAnchorPoint(Vec2(0.44f, 0.5f));
	else if(direction == GameObject::Direction::RIGHT)
		_sprite->setAnchorPoint(Vec2(0.58f, 0.5f));
	else
		_sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
	//

	setGridPosition(getGridPosition() + directionToOffset(direction));
	setGridDirection(direction);
}

void Projectile::onMoved()
{
	auto scene = _engine->getCurrentScene<GameplayScene>();
	if (scene == nullptr)
		return;

	auto newPos = getGridPosition() + directionToOffset(_direction);
	if (!scene->getGrid()->isValidTileCoord(newPos)) {
		remove();
		return;
	}

	auto obj = scene->findGameObject(newPos);
	if (obj != nullptr) {
		if (onCollidedWithGameObject(_direction, obj)) {
			remove();
			return;
		}
	}

	auto value = scene->getGrid()->getTile(newPos);
	if (onCollidedWithBlock(_direction, value)) {
		remove();
		return;
	}

	move(_direction);
	return;
}

bool Projectile::onCollidedWithBlock(Direction direction, const TileValue& value)
{
	if (value == TileValue::BLOCK) {
		//_engine->playSound("shot_into_wall");
		return true;
	}

	return false;
}

bool Projectile::onCollidedWithGameObject(Direction direction, GameObject* obj)
{
	auto tank = dynamic_cast<Tank*>(obj);
	if (tank != nullptr) {
		if (!tank->isGhostState() && !tank->isFallIntoHoleState()) {
			tank->damage();
			return true;
		}
	}

	auto bonus = dynamic_cast<Bonus*>(obj);
	if (bonus != nullptr) {
		bonus->kill();
		return true;
	}

	return false;
}

void Projectile::remove()
{
	auto scene = _engine->getCurrentScene<GameplayScene>();
	if (scene != nullptr)
		scene->removeGameObject(this);
}