# Graphillion 2.0 のクラス

[English version](graphillion2_0.md)

Graphillion 2.0 では GraphSet クラス以外に、いくつかクラスが追加されました。

GraphSet: 辺の集合の集合によって部分グラフの集合を表すクラス
VertexSetSet: 頂点の集合の集合を表すクラス
DiGraphSet: 有向辺の集合の集合によって有向グラフの部分グラフの集合を表すクラス
EdgeVertexSetSet: 辺と、辺が誘導する頂点の集合の集合によって部分グラフの集合を表すクラス

表記の一貫性のため、GraphSet には EdgeSetSet という別名が付与されており、GraphSet と EdgeSetSet のどちらの名前も使用できます。


# Universe について

旧バージョンの Graphillion では、GraphSet.set_universe() メソッドによって、Universe（台グラフ）を設定することができました。Graphillion でも引き続き GraphSet.set_universe() メソッドを使用することができますが、GraphSet、VertexSetSet、EdgeVertexSetSet に共通の Universe を、Universe.set_universe() メソッドを使用することで設定することができます。GraphSet.set_universe() を Universe.set_universe() に置き換えるだけで、既存の旧バージョンに向けて書かれたコードはそのまま動きます。例えば、台グラフの頂点集合を V = {1,2,3}, 辺集合を E = {(1,2),(1,3)} とすると、Universe は以下の通りに設定します。

```
Universe.set_universe([(1,2),(1,3)], order = 'as-is')
```

このメソッド呼び出しにより、GraphSet の Universe は [(1,2),(1,3)]、VertexSetSet の Universe は [2,1,3]、EdgeVertexSetSet の Universe は [(1,2),2,(1,3),1,3] に設定されます。VertexSetSet と EdgeVertexSetSet の Universe の順序は自動で設定されます。ほとんどの場合で、VertexSetSet や EdgeVertexSetSet の Universe は自動で設定された順序のままで問題は生じないはずです。VertexSetSet については、VertexSetSet.set_universe() メソッドを呼び出すことで、手動で Universe の順序を設定できます。EdgeVertexSetSet の順序の手動設定は、現バージョンでは未対応です。

DiGraphSet の Universe は、GraphSet の Universe とは独立です。GraphSet に設定した Universe とは無関係に DiGraphSet の Universe を設定できます。DiGraphSet.set_universe() メソッド、または、Universe.set_digraph_universe() によって設定できます。設定例:

```
GraphSet.set_universe([(1,2), (2,3), (3,4)])
DiGraphSet.set_universe([(1,2), (1,3), (2,1)]) # GraphSet とは独立に設定可能
```

無向の場合（GraphSet）とは異なり、(1,2) と (2,1) のように2つの端点の両方向の有向辺を台グラフに含めることができます。

DiGraphSet に対応する、頂点の集合の集合や、辺の集合の集合を表すクラスは現バージョン（Graphillion 2.0）では存在しません。


# VertexSetSet で台グラフが必要になる場合

例えば、VertexSetSet クラスでは、独立集合の集合を生成することが可能です。現在のバージョン Graphillion 2.0 では、Universe で設定したグラフと同じグラフを、独立集合の集合の生成の際に手動で設定する必要があります。将来のバージョンでは、引数にグラフを指定する必要はなくなる予定です。

```
# グラフ [(1,2),(1,3)] のすべての独立集合の集合を求める。
# 引数にグラフを指定する必要がある
# 事前に Universe.set_universe([(1,2),(1,3)]) を呼び出して、VertexSetSet の Universe を設定する必要があることに注意。
vss = VertexSetSet.independent_setsets([(1,2),(1,3)])
```

Universe.set_universe では、VertexSetSet については、頂点集合だけが Universe として設定されます。
そのため、Universe.set_universe で設定する台グラフと、VertexSetSet.independent_setsets() の引数のグラフは
異なるものに設定することが可能ですが、混乱を招くため、原則として両者には同じグラフを設定するのが無難です。


# 孤立した頂点について

（Graphillion 2.0 ではまだ対応していません。将来のバージョンで対応予定です。）

EdgeVertexSetSet クラスでは、孤立した頂点（次数が0の頂点）をもつ台グラフを扱うことができます。例えば、
V = {1,2,3,4,5,6}, E = {(1,2),(1,3)} としたとき、頂点4,5,6は孤立した頂点です。GraphSet では、辺の集合の集合で部分グラフを扱うため、
孤立した頂点は意味がないため、旧バージョンでは設定できませんでした。
（例えば、GraphSet.trees は木の集合を返すメソッドですが、木は辺の集合で表されるため、
頂点1つだけからなる ({4}, {}) のような木を表すことはできませんでした。）
孤立した頂点は、set_universe メソッドの isolated 引数で設定することができます。
isolated 引数には、孤立頂点のリストを与えます。

```
Universe.set_universe([(1,2),(1,3)], isolated = [4, 5, 6], order = 'as-is')
```

GraphSet や DiGraphSet 使用の際は、isolated に設定された孤立頂点は無視されます。（上記の例では [4,5,6] は無いものとみなされます。）
VertexSetSet、EdgeVertexSetSet では、Universe は孤立頂点も含めて設定されます。
上記の例では、VertexSetSet の Universe は [2,1,3,4,5,6]、EdgeVertexSetSet の Universe は [(1,2),2,(1,3),1,3,4,5,6] に設定されます。

例えば、以下の例では、孤立した頂点を含む台グラフ上で独立集合の集合を求めています。

```
vss = VertexSetSet.independent_setsets([(1,2),(1,3)], isolated = [4,5,6])
```

vss は、[2,3,4] や [2,3,4,5,6]、[2,3] などの独立集合を含みます。

# EdgeVertexSetSet について

EdgeVertexSetSet は、部分グラフの集合を扱うためのクラスですが、各部分グラフは、
辺と、辺に含まれる頂点の集合によって構成されます。
例えば、Universe が V = {1,2,3,4,5,6}, E = {(1,2),(1,3)} であるとき、
[(1,2),1,2] や [(1,2),1,2,3], [(1,2),1,2,4] は EdgeVertexSetSet の要素になることができますが、
[(1,2),1] や [(1,2)] は EdgeVertexSetSet の要素になることができません（前者の例では
頂点 2 が含まれておらず、後者の例では頂点 1,2 が含まれていないため）。

現バージョン（Graphillion 2.0）では、EdgeVertexSetSet は限定された機能だけを持ちます。
EdgeVertexSetSet の構築の主な方法は、GraphSet クラスからの変換です。
以下の例では、パスの集合を、パスを構成する辺と辺に含まれる頂点の集合からなる集合に変換します。

```
gs = GraphSet.paths(...)
evss = gs.to_edgevertex_setset() # GraphSet クラスから EdgeVertexSetSet に変換
```

evss を用いることで、例えば、頂点と辺の両方に重みが付与されているグラフの最適化問題を解くことができます。



# Reconfillion について

Reconfillion は Graphillion を用いて組合せ遷移問題（Combinatorial reconfiguration problem）を解くための
ライブラリです。Graphillion の新バージョンから、Reconfillion に対応しています。

組合せ遷移問題とは、ある組合せ問題と、組合せ問題の2つの解、遷移ルールが与えられた際に、
遷移ルールに基づいて、一方の解を遷移させながら、組合せ問題の条件を満たしつつ、他方の解に遷移できるかを
判定する問題です。例えば、独立集合遷移問題では、グラフ G と、独立集合が2つ与えられます。
遷移ルールはいくつか考えることができますが、最も有名な遷移ルールは、トークンジャンピングと呼ばれるルールで、
ある独立集合 S の1つの頂点を削除し、S に含まれていない別の頂点を加えるというルールです。

Graphillion では、実行可能解の集合（上の例の場合はグラフ G のすべての独立集合の集合）を構築することが
可能で、それを用いて組合せ遷移問題を解くことができます。遷移ルールとしては、上記のトークンジャンピングを
含めて、様々な遷移ルールに対応しています。これにより、Graphillion で扱うことのできる対象であれば、
それに対応する組合せ遷移問題を解くことが可能になります。


以下のコードにより、組合せ遷移問題を解く（解となる遷移列を求める）ことが可能です。

```
graph = [(1, 2), (1, 3), (1, 4), (2, 3), (2, 4), (3, 4)]

from graphillion import GraphSet
from reconfillion import reconf

GraphSet.set_universe(graph)
spanning_trees = GraphSet.trees(is_spanning = True)

s = [(1, 2), (1, 3), (1, 4)]
t = [(1, 4), (2, 4), (3, 4)]

reconf_sequence = reconf.get_reconf_seq(s, t, spanning_trees, model = 'tj')
```

以下の例では、遷移列を複数求めます。

```
reconf_sequences = reconf.get_reconf_seq_multi(s, t, spanning_trees, model = 'tj')

for i in reconf_sequences:
    print(i)
```

reconf_sequences はイテレータオブジェクトです。一般に、遷移列の数は組合せ爆発のため
莫大になるため、上記のように、すべてを列挙して処理することは困難です。
reconf_sequences から、必要な数だけを取り出すようにしてください。

組合せ遷移問題に対応しているクラスは、GraphSet, DiGraphSet, VertexSetSet, setset クラスです。
GraphSet の場合、例えば、パス、全域木、マッチング、シュタイナー木などの遷移問題に対応しています
（GraphSet が扱えるすべてのグラフクラスに対応しています）。
VertexSetSet の場合、例えば、独立集合、支配集合、頂点被覆、横断集合などの遷移問題に対応しています。
setset クラスは要素の集合の集合を表すクラスです。setset クラスのオブジェクトを構築できれば、
様々な組合せ遷移問題を解くことができます。例えば、以下の例では、ナップサック遷移問題を解く例です。

```
ss = setset({}) # べき集合
weights = {...} # 要素の重み
ss2 = ss.cost_le(weights, b) # 重みが b 以下の集合のみを抽出
# 解空間 ss2 を用いると遷移問題を解くことができる
```



# Graphillion 2.0 における並列計算について

Graphillion の旧バージョンで並列計算に対応していましたが、Graphillion 2.0 では、
新たに、使用するスレッドの数を制御することができるようになりました。
内部で、OpenMP の対応する関数/メソッドを呼び出しているため、詳細は、
OpenMP のマニュアルを参照してください。

omp_get_max_threads(): 最大スレッド数を取得
omp_get_num_threads(): 現在のスレッド数を取得
omp_set_num_threads(): スレッド数を設定
omp_get_num_procs(): CPU数を設定


# 謝辞

本ライブラリの一部は、JSPS科研費 JP18H04091, JP20H05794, JP23H04383 の研究として開発されました。







