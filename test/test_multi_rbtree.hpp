#ifndef TEST_MULTI_RBTREE_HPP
#define TEST_MULTI_RBTREE_HPP

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>

#include "../src/rbtree.hpp"

using namespace ygg;

#define TESTSIZE 1000

class TAG_A {};
class TAG_B {};

namespace multitest {

class Node : public RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>, TAG_A>, public RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>, TAG_B>
{ // No multi-nodes!
public:
	int dataA;
	int dataB;

	Node()
					: dataA(0), dataB(0)
	{};

	explicit Node(int dataA_in, int dataB_in)
					: dataA(dataA_in), dataB(dataB_in)
	{};

	Node(const Node &other)
					: dataA(other.dataA), dataB(other.dataB)
	{};
};

using NodeBaseA = RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>, TAG_A>;
using NodeBaseB = RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>, TAG_B>;

class CompareA
{
public:
	bool operator()(const Node &lhs, const Node &rhs) const
	{
		return lhs.dataA < rhs.dataA;
	}
};

class CompareB
{
public:
	bool operator()(const Node &lhs, const Node &rhs) const
	{
		return lhs.dataB < rhs.dataB;
	}
};

class NodeTraits
{
public:
	static std::string get_id(const Node *node)
	{
		return std::to_string(node->dataA) + std::string (" / ") + std::to_string(node->dataB);
	}

	static void leaf_inserted(Node &node)
	{ (void)node; };

	static void rotated_left(Node &node)
	{ (void)node; };

	static void rotated_right(Node &node)
	{ (void)node; };

	static void delete_leaf(Node &node)
	{ (void)node; };

	static void swapped(Node &n1, Node &n2)
	{
		(void)n1;
		(void)n2;
	};
};

using TreeA = RBTree<Node, NodeTraits, TreeOptions<TreeFlags::MULTIPLE>, TAG_A, CompareA>;
using TreeB = RBTree<Node, NodeTraits, TreeOptions<TreeFlags::MULTIPLE>, TAG_B, CompareB>;

TEST(MultiRBTreeTest, TrivialInsertionTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	Node n(0, 0);
	ta.insert(n);
	tb.insert(n);

	ASSERT_TRUE(ta.verify_integrity());
	ASSERT_TRUE(tb.verify_integrity());
}

TEST(MultiRBTreeTest, MinimalInteractionTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

  class TestNode : public Node
  {
  public:
	  using Node::Node;
  private:
	  FRIEND_TEST(MultiRBTreeTest, MinimalInteractionTest);
  };

	TestNode n1(0, 0);
	TestNode n2(-1, 1);

	ta.insert(n1);
	ta.insert(n2);

	ASSERT_EQ(n1.NodeBaseA::_rbt_right, nullptr);
	ASSERT_EQ(n1.NodeBaseA::_rbt_left, &n2);
	ASSERT_EQ(n1.NodeBaseA::get_parent(), nullptr);
	ASSERT_EQ(n2.NodeBaseA::get_parent(), &n1);

	tb.insert(n1);
	tb.insert(n2);

	ASSERT_EQ(n1.NodeBaseA::_rbt_right, nullptr);
	ASSERT_EQ(n1.NodeBaseA::_rbt_left, &n2);
	ASSERT_EQ(n1.NodeBaseA::get_parent(), nullptr);
	ASSERT_EQ(n2.NodeBaseA::get_parent(), &n1);

	ASSERT_EQ(n1.NodeBaseB::_rbt_right, &n2);
	ASSERT_EQ(n1.NodeBaseB::_rbt_left, nullptr);
	ASSERT_EQ(n1.NodeBaseB::get_parent(), nullptr);
	ASSERT_EQ(n2.NodeBaseB::get_parent(), &n1);

	ASSERT_TRUE(ta.verify_integrity());
	ASSERT_TRUE(tb.verify_integrity());
}

TEST(MultiRBTreeTest, RandomInsertionTest) {
  auto ta = TreeA();
	auto tb = TreeB();

  std::mt19937 rng(4); // chosen by fair xkcd
  std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
                                         std::numeric_limits<int>::max());

  Node nodes[TESTSIZE];


  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    int val_a = uni(rng);
	  int val_b = uni(rng);

    nodes[i] = Node(val_a, val_b);

    ta.insert(nodes[i]);
		tb.insert(nodes[i]);

    //std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(ta.verify_integrity());
	  ASSERT_TRUE(tb.verify_integrity());
  }

	int last_val = std::numeric_limits<int>::min();
	for (const auto & n : ta) {
		ASSERT_GE(n.dataA, last_val);
		last_val = n.dataA;
	}

	last_val = std::numeric_limits<int>::min();
	for (const auto & n : tb) {
		ASSERT_GE(n.dataB, last_val);
		last_val = n.dataB;
	}
}

TEST(MultiRBTreeTest, LinearInsertionTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	Node nodes[TESTSIZE];

	for (unsigned int i = 0; i < TESTSIZE; ++i) {
		nodes[i] = Node(i, i);

		ta.insert(nodes[i]);
		tb.insert(nodes[i]);

		//std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i) + std::string(".dot");
		//tree.dump_to_dot(fname);

		ASSERT_TRUE(ta.verify_integrity());
		ASSERT_TRUE(ta.verify_integrity());
	}
}

/*
TEST(RBTreeTest, HintedPostEqualInsertionTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode n_insert_before(1, 0);
  EqualityNode n_pre(1, 1);
  EqualityNode n_insert_between(1, 2);
  EqualityNode n_post(2, 3);

  tree.insert(n_pre);
  tree.insert(n_post);

  ASSERT_TRUE(tree.verify_integrity());

  // should be inserted before pre
  tree.insert(n_insert_before, n_pre);

  // should be inserted between pre and post
  tree.insert(n_insert_between, n_post);


  auto it = tree.begin();
  ASSERT_EQ(it->sub_data, 0);
  it++;
  ASSERT_EQ(it->sub_data, 1);
  it++;
  ASSERT_EQ(it->sub_data, 2);
  it++;
  ASSERT_EQ(it->sub_data, 3);
  it++;
  ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, RepeatedHintedPostEqualInsertionTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode nodes_pre[TESTSIZE];
  EqualityNode nodes_post[TESTSIZE];
  EqualityNode nodes_between[TESTSIZE];
  EqualityNode node_border_small(1, TESTSIZE + 2);
  EqualityNode node_border_large(2, TESTSIZE + 2);

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes_pre[i] = EqualityNode(1, i);
    nodes_post[i] = EqualityNode(2, i);
    nodes_between[i] = EqualityNode(1, TESTSIZE + 1);
  }

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    tree.insert(nodes_post[i], tree.end()); // insert in order
  }

  tree.insert(nodes_pre[TESTSIZE-1], nodes_post[0]);

  for (int i = TESTSIZE - 2 ; i >= 0 ; --i) {
    tree.insert(nodes_pre[i], nodes_pre[i+1]);
    ASSERT_EQ(tree.begin()->sub_data, i);
  }

  for (int i = 0 ; i < TESTSIZE ; ++i) {
    tree.insert(nodes_between[i], nodes_pre[i]);
  }

  tree.insert(node_border_large, nodes_post[0]);
  tree.insert(node_border_small, node_border_large);
  ASSERT_TRUE(tree.verify_integrity());

  auto it = tree.begin();
  for (int i = 0 ; i < TESTSIZE ; ++i) {
    ASSERT_EQ(it->data, 1);
    ASSERT_EQ(it->sub_data, TESTSIZE + 1); // first, the 'between' node
    it++;
    ASSERT_EQ(it->data, 1);
    ASSERT_EQ(it->sub_data, i); // now, the pre-node
    it++;
  }

  ASSERT_EQ(it->data, 1);
  ASSERT_EQ(it->sub_data, TESTSIZE + 2); // small border
  it++;
  ASSERT_EQ(it->data, 2);
  ASSERT_EQ(it->sub_data, TESTSIZE + 2); // large border
  it++;

  for (int i = 0 ; i < TESTSIZE ; ++i) {
    ASSERT_EQ(it->data, 2);
    ASSERT_EQ(it->sub_data, i); // post-nodes
    it++;
  }
}

TEST(RBTreeTest, LinearEndHintedInsertionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);
  }

  tree.insert(nodes[TESTSIZE - 1]);

  for (int i = TESTSIZE - 2 ; i >= 0 ; --i) {
    tree.insert(nodes[i], nodes[TESTSIZE - 1]);
    ASSERT_TRUE(tree.verify_integrity());
  }

  int i = 0;
  for (const auto & n : tree) {
    ASSERT_EQ(n.data, i);
    i++;
  }
}

TEST(RBTreeTest, HinterOrderPreservationTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode nodes[3*TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[3*i] = EqualityNode(i, 0);
    nodes[3*i + 1] = EqualityNode(i, 1);
    nodes[3*i + 2] = EqualityNode(i, 2);
  }

  // insert the middles
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    tree.insert(nodes[3*i + 1]);
  }

  tree.verify_integrity();

  // insert the prefix, using a hint
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    tree.insert(nodes[3*i], nodes[3*i + 1]);
  }

  tree.verify_integrity();

  // insert the postfix, using a hint
  for (unsigned int i = 0 ; i < TESTSIZE - 1 ; ++i) {
    tree.insert(nodes[3*i + 2], nodes[3*i + 3]);
  }

  unsigned int i = 0;
  for (auto & n : tree) {
    ASSERT_EQ(n.data, i / 3);
    ASSERT_EQ(n.sub_data, i % 3);
    ++i;
  }
}

TEST(RBTreeTest, LinearNextHintedInsertionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);
  }

  tree.insert(nodes[TESTSIZE - 1]);

  for (int i = TESTSIZE - 2 ; i >= 0 ; --i) {
    tree.insert(nodes[i], nodes[i+1]);
    ASSERT_TRUE(tree.verify_integrity());
  }

  int i = 0;
  for (const auto & n : tree) {
    ASSERT_EQ(n.data, i);
    i++;
  }
}

TEST(RBTreeTest, UpperBoundTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(2*i);
    tree.insert(nodes[i]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < TESTSIZE - 1 ; ++i) {
    Node query(2*i + 1);
    auto it = tree.upper_bound(query);
    ASSERT_EQ(it->data, nodes[i+1].data);
  }

  Node query(2*(TESTSIZE - 1) + 1);
  auto it = tree.upper_bound(query);
  ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, TrivialDeletionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node n1;
  n1.data = 0;
  tree.insert(n1);

  Node n2;
  n2.data = 1;
  tree.insert(n2);

  ASSERT_TRUE(tree.verify_integrity());

  tree.remove(n2);

  ASSERT_TRUE(tree.verify_integrity());

  tree.remove(n1);

  ASSERT_TRUE(tree.verify_integrity());
}

TEST(RBTreeTest, LinearInsertionLinearDeletionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);

    tree.insert(nodes[i]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    //std::cout << "\n\n Removing " << i << "\n";
    tree.remove(nodes[i]);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, LinearInsertionRandomDeletionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[TESTSIZE];
  std::vector<unsigned int> indices;

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);

    tree.insert(nodes[i]);
    indices.push_back(i);
  }

  std::mt19937 rng(4); // chosen by fair xkcd
  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    //std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    tree.remove(nodes[indices[i]]);

    //fname = std::string("/tmp/trees/after-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, LinearMultipleIterationTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode nodes[TESTSIZE*5];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    for (unsigned j = 0 ; j < 5 ; ++j) {
      nodes[5*i + j] = EqualityNode(i);
      tree.insert(nodes[5*i + j]);
    }
  }

  ASSERT_TRUE(tree.verify_integrity());

  unsigned int i = 0;
  for (auto & n : tree) {
    ASSERT_EQ(n.data, nodes[i].data);
    i++;
  }
}
*/


TEST(MultiRBTreeTest, LinearIterationTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	Node nodes[TESTSIZE];

	std::vector<size_t> indices;
	for (unsigned int i = 0; i < TESTSIZE; ++i) {
		nodes[i] = Node(i, TESTSIZE - i);
		indices.push_back(i);
	}

	std::mt19937 rng(4); // chosen by fair xkcd
	std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
		std::uniform_int_distribution<unsigned int> uni(0,
		                                                i - 1);
		return uni(rng);
	});

	for (auto index : indices) {
		ta.insert(nodes[index]);
		tb.insert(nodes[index]);
	}

	ASSERT_TRUE(ta.verify_integrity());
	ASSERT_TRUE(tb.verify_integrity());

	unsigned int i = 0;
	for (auto &n : ta) {
		ASSERT_EQ(n.dataA, i);
		i++;
	}

	i = 1;
	for (auto &n : tb) {
		ASSERT_EQ(n.dataB, i);
		i++;
	}
}

/*
TEST(RBTreeTest, ReverseIterationTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[TESTSIZE];
  std::vector<size_t> indices;
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);
    indices.push_back(i);
  }

  std::mt19937 rng(4); // chosen by fair xkcd
  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  auto it = tree.rbegin();
  unsigned int i = TESTSIZE - 1;
  while (it != tree.rend()) {
    ASSERT_EQ(it->data, i);
    it++;
    i--;
  }
}

TEST(RBTreeTest, FindTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(2*i);
    tree.insert(nodes[i]);
  }

  // Nonexisting
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    Node findme(2*i+1);
    auto it = tree.find(findme);
    ASSERT_EQ(it, tree.cend());
  }

  // Existing
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    Node findme(2*i);
    auto it = tree.find(findme);
    ASSERT_EQ(&(*it), &(nodes[i]));
  }
}

TEST(RBTreeTest, ComprehensiveTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node persistent_nodes[TESTSIZE];
  std::vector<unsigned int> indices;
  std::mt19937 rng(4); // chosen by fair xkcd

  std::set<unsigned int> values_seen;

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    unsigned int data = 10 * i;
    persistent_nodes[i] = Node(data);
    indices.push_back(i);
    values_seen.insert(data);
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(persistent_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  Node transient_nodes[TESTSIZE];
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> uni(0, 10 * (TESTSIZE + 1));
    unsigned int data = uni(rng);

    while (values_seen.find(data) != values_seen.end()) {
      data = uni(rng);
    }

    transient_nodes[i] = Node(data);

    values_seen.insert(data);

    //std::cout << "Inserting random value: " << data << "\n";
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(transient_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  //std::string fname_before = std::string("/tmp/trees/rbt-comprehensive-before.dot");
  //tree.dump_to_dot(fname_before);

  for (int i = 0 ; i < TESTSIZE ; ++i) {
    tree.remove(transient_nodes[i]);


    //std::string rem_fname = std::string("/tmp/trees/removed-") + std::to_string(i) + std::string(".dot");
    //std::cout << "Step " << i << ": removing data " << transient_nodes[i].data << "\n";
    //tree.dump_to_dot(rem_fname);
    ASSERT_TRUE(tree.verify_integrity());
  }

  //std::string fname = std::string("/tmp/trees/rbt-comprehensive.dot");
  //tree.dump_to_dot(fname);

  // Query elements
  for (int i = 0 ; i < TESTSIZE ; ++i) {
    auto it = tree.find(persistent_nodes[i]);
    assert(&(*it) == &(persistent_nodes[i]));
    ASSERT_EQ(&(*it), &(persistent_nodes[i]));
  }

}


TEST(RBTreeTest, ComprehensiveMultipleTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode persistent_nodes[TESTSIZE];
  std::vector<unsigned int> indices;
  std::mt19937 rng(4); // chosen by fair xkcd

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    unsigned int data = 10 * i;
    persistent_nodes[i] = EqualityNode(data);
    indices.push_back(i);
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(persistent_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  EqualityNode transient_nodes[TESTSIZE];
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> uni(0, 10 * (TESTSIZE + 1));
    unsigned int data = uni(rng);

    transient_nodes[i] = EqualityNode(data);
    //std::cout << "Inserting random value: " << data << "\n";
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(transient_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  //std::string fname_before = std::string("/tmp/trees/rbt-comprehensive-before.dot");
  //tree.dump_to_dot(fname_before);

  for (int i = 0 ; i < TESTSIZE ; ++i) {
    tree.remove(transient_nodes[i]);

    //std::string rem_fname = std::string("/tmp/trees/removed-") + std::to_string(i) + std::string(".dot");
    //std::cout << "Step " << i << ": removing data " << transient_nodes[i].data << "\n";
    //tree.dump_to_dot(rem_fname);
    ASSERT_TRUE(tree.verify_integrity());
  }


  //std::string fname = std::string("/tmp/trees/rbt-comprehensive.dot");
  //tree.dump_to_dot(fname);

  // Query elements
  for (int i = 0 ; i < TESTSIZE ; ++i) {
    //std::cout << "Finding " << i << "\n";
    auto it = tree.find(persistent_nodes[i]);
    assert(&(*it) == &(persistent_nodes[i]));
    ASSERT_EQ(&(*it), &(persistent_nodes[i]));
  }

}

 */
// TODO test equal elements

} // namespace multitest

#endif // TEST_RBTREE_HPP
