#include "GameObject.h"
#include "Transform.h"
#include <algorithm>

/* STATIC VARIABLES and FUNCTIONS*/
vector<GameObject*> GameObject::gameObjects;

GameObject* GameObject::Find(const string& path) {
	for (auto obj : gameObjects)
	{
		if (obj->getName() == path) return obj;
	}
	return nullptr;
}

void GameObject::Add(GameObject* obj) {
	if (obj == nullptr) return;
	gameObjects.push_back(obj);
}

void GameObject::Remove(GameObject* obj) {
	//using iterator
	if (obj == nullptr) return;

	//for each는 erase처럼 변화가 있을 경우에는 사용불가
	for (auto it = gameObjects.begin(); it != gameObjects.end(); ) {
		auto o = *it;
		if (o == obj) {
			it = gameObjects.erase(it); //삭제 후의 상태로 update
			delete o;
		}
		else { it++; }
	}

	////erase-remove idiom //범위기반 삭제
	//gameObjects.erase(std::remove_if(
	//	gameObjects.begin(),	//처음부터
	//	gameObjects.end(),		//끝까지
	//	[&](GameObject* item) { return obj == item; }
	//), gameObjects.end()); //end는 마지막 element 다음인 끝을 의미한다.
}

/* General variables and functions */

GameObject::GameObject(const string& name, 
	GameObject* parent, const string& tag, 
	const string& shape, const Vector2& pos)
	: name(name), tag(tag), enabled(true), parent(parent),
	transform(new Transform(this, shape, pos) ) {
	components.clear();
	components.push_back(transform);
}

GameObject::~GameObject() {}

void GameObject::traverseStart() {
	if (enabled == false) return;

	for (auto comp : components)
	{
		comp->start();
	}
	for (auto child : children)
	{
		child->traverseStart();
	}
}

void GameObject::traverseUpdate() {
	if (enabled == false) return;

	for (auto comp : components)
	{
		comp->update();
	}
	for (auto child : children)
	{
		child->traverseUpdate();
	}
}