#include "MyAccessHub.h"

//staticメンバ変数は初期化が必要
//C++はなんと宣言部の外に書く必要がある。C#と大きく異なる
//複数のcppで読み込むヘッダに初期化を書くとリンクエラーになるので注意
//初期化の際は.cpp側への記述がおすすめ
MyGameEngine* MyAccessHub::m_engine = nullptr;