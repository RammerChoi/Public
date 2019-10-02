#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>

using namespace std;

struct Node
{
	Node* pFail_ = nullptr;
	map<char, Node> edges_;
	bool isCompleted_ = false;

	char c_ = 0;

	Node(const char _c)
		: c_(_c)
	{
	};

	void Insert(const string& _str, const int _checkIdx = 0)
	{
		if (_str.size() <= _checkIdx)
		{
			isCompleted_ = true;
			return;
		}

		const char c = _str[_checkIdx];
		auto it = edges_.try_emplace(c, c).first;
		it->second.Insert(_str, _checkIdx + 1);
	}

	bool Find(const string& _str, const int _checkIdx = 0) const
	{
		if (_str.size() <= _checkIdx)
			return false;

		const char c = _str[_checkIdx];
		auto it = edges_.find(c);
		if (it == edges_.end())
		{
			if (pFail_)
				return pFail_->Find(_str, _checkIdx);

			return Find(_str, _checkIdx + 1);
		}

		if (it->second.isCompleted_)
			return true;

		return it->second.Find(_str, _checkIdx + 1);
	}

	Node* GetChild(char _c)
	{
		auto it = edges_.find(_c);
		if (it == edges_.end())
			return nullptr;

		return &it->second;
	}

	void Build()
	{
		// failLink build
		queue<Node*> q;
		q.push(this); // this == root
		while (!q.empty())
		{
			Node* pCur = q.front();
			q.pop();
			if (pCur == nullptr)
				continue;

			for (auto& childPair : pCur->edges_)
			{
				char targetChar = childPair.first;
				Node* pNext = &childPair.second;
				if (pCur == this)
				{
					pNext->pFail_ = this;
					q.push(pNext);
					continue;
				}

				Node* pBack = pCur->pFail_;
				Node* pBackChild = pBack->GetChild(targetChar);
				while (pBackChild == nullptr &&
					pBack != this)
				{
					pBack = pBack->pFail_;
					pBackChild = pBack->GetChild(targetChar);
				}

				if (pBackChild)
				{
					pBack = pBackChild;
				}

				pNext->pFail_ = pBack;

				if (pNext->pFail_->isCompleted_)
				{
					pNext->isCompleted_ = true;
				}

				q.push(pNext);
			}
		}
	}
};

// 알파벳이라고 생각하지 않고 유니코드 문자라고 생각한다
int main()
{
	int keywordNum = 0;
	cin >> keywordNum;

	Node root(0);
	string keyword;
	for (int i = 0; i < keywordNum; ++i)
	{
		cin >> keyword;
		root.Insert(keyword);
	}

	root.Build();

	int testNum = 0;
	cin >> testNum;

	string testStr;
	for (int t = 0; t < testNum; ++t)
	{
		cin >> testStr;
		if (root.Find(testStr))
		{
			cout << "YES" << endl;
		}
		else
		{
			cout << "NO" << endl;
		}
	}
}
