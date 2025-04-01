# Graphillion ソフトウェアのバージョン0.99から2.0までの主要な変更点

- **新しいクラスの追加**
  - 頂点集合の集合を扱う VertexSetSet クラス
  - 有向グラフの集合を扱う DiGraphSetクラス（DiGraphillionからマージ）
  - 辺と頂点の両方で表現される部分グラフの集合を扱う EdgeVertexSetSet クラス

- **頂点集合の集合を扱う VertexSetSet クラスの追加**
  - 標準的なセット演算を実装: 和集合、積集合、差集合、対称差、商、剰余、補集合、結合、交叉
  - グラフ特有の演算を実装: 部分グラフ、上位グラフ、非部分グラフ、非上位グラフ
  - 頂点または頂点セットを追加、削除、破棄するメソッドを提供
  - 頂点の存在を切り替えるflip操作を実装
  - 特定の頂点を含む/含まないセットをフィルタリングするincluding/excluding操作を提供
  - 等価性、部分集合、上位集合、素集合のチェックでセットを比較
  - smaller()、larger()、graph_size()、len()メソッドでサイズによるフィルタリング
  - dump/loadとdumps/loadsによるファイルまたは文字列への/からのセットの保存/読み込みをサポート
  - 様々なイテレータを提供: 標準、ランダム、最小コスト、最大コスト
  - 極小/極大セットを見つけるためのminimal()とmaximal()操作を提供
  - 頂点コストでフィルタリングするcost_le()、cost_ge()、cost_eq()を実装
  - 重み付き頂点操作をサポート
  - 1つの頂点を削除、追加、削除＆追加したセットを返す remove_some_vertex()、add_some_vertex()、remove_add_some_vertices()
  - independent_sets()、dominating_sets()、vertex_covers()、cliques(): グラフ内のすべての独立集合、支配集合、頂点被覆、クリークの集合を計算
  - to_vertexsetset()を使用してGraphSetとVertexSetSet間の変換を行う

- **有向グラフの集合を扱う DiGraphSetクラス（DiGraphillionからマージ）**
  - GraphSet や VertexSetSet と同様の、多くの集合演算をサポート
  - directed_cycles(): グラフ内のすべての有向サイクルの集合を返す
  - directed_hamiltonian_cycles(): すべての有向ハミルトン閉路の集合を返す
  - directed_st_paths(): 始点sから終点tへのすべての有向パスの集合を返す
  - rooted_forests(): すべての根付き森（指定された根を持つ場合もあり）の集合を返す
  - rooted_trees(): 指定された根ノードを持つすべての根付き木の集合を返す
  - rooted_spanning_forests(): 指定された根ノードを持つすべての全域森の集合を返す
  - rooted_spanning_trees(): 指定された根ノードを持つすべての全域木の集合を返す

- **辺と頂点の両方で表現される部分グラフの集合を扱う EdgeVertexSetSet クラス**
  - GraphSet や VertexSetSet と同様の、多くの集合演算をサポート
  - to_edgevertexsetset() を使用してGraphSetとEdgeVertexSetSet間の変換を行う

- **ユニバース管理の改善**
  - GraphSet、VertexSetSet、EdgeVertexSetSetのユニバース管理をUniverseクラスに統合
  - VertexSetSet.set_universe()の呼び出しが不要になるよう修正


- **グラフ作成メソッドの拡張**
  - bicliques
  - matchings, perfect_matchings, k_matchings, b_matchings
  - k_factors, f_factors
  - regular_graphs, regular_bipartite_graphs
  - steiner_subgraphs, steiner_trees, steiner_cycles, steiner_paths
  - degree_distribution_graphs
  - letter_P_graphs
  - induced_graphs, weighted_induced_graphs
  - chordal_graphs, bipartite_graphs
  - partitions, balanced_partitions
  - Claw graphs and claw-free graphs
  - Diamond graphs and diamond-free graphs
  - Gem graphs and gem-free graphs
  - Odd hole graphs and odd-hole-free graphs
  - Cographs
  - Chordal bipartite graphs
  - Split graphs
  - Block graphs
  - Ptolemaic graphs
  - Threshold graphs
  - Gridline graphs
  - Domino graphs
  - Linear domino graphs

- **グラフ操作の強化**
  - コストベースのフィルタリングメソッドの追加:
    - cost_le(): 総辺コストが特定の境界以下のグラフを返す
    - cost_ge(): 総辺コストが特定の境界以上のグラフを返す
    - cost_eq(): 総辺コストが特定の値と等しいグラフを返す
  - グラフ修正メソッドの追加:
    - remove_some_edge(): 1つの辺を削除したグラフの集合を返す
    - add_some_edge(): 1つの辺を追加したグラフの集合を返す
    - remove_add_some_edges(): 1つの辺を削除し別の辺を追加したグラフの集合を返す
  - reliability() メソッドによるネットワーク信頼性計算の追加

- **Python言語サポートの変更**
  - Python 3のサポート追加
  - Python 2.7のサポート終了
  - 文字列エラーの代わりに例外クラスを使用するよう構文を修正

- **パフォーマンスの向上**
  - 新しい「greedy」アルゴリズムによる辺順序探索の改善
  - 適切なpickleプロトコルによるシリアル化/逆シリアル化の強化
  - グラフ生成メソッドのためのOpenMP並列処理サポートの追加

- **バグ修正と保守**
  - マルチスレッド環境でのバグ修正
  - GraphSetとsetsetクラスの区別（両方が独立して使用可能に）
  - graphsetと文字列表現間の変換問題の修正
  - イテレータメソッドにおけるStopIterationの例外処理の改善
  - NetworkX 2.0のサポート追加
  - PyPI 規約に従い、のパッケージ名の小文字化
  - Pythonパッケージング用の pyproject.toml の追加



