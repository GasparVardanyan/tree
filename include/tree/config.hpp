# ifndef TREE_CONFIG_H
# define TREE_CONFIG_H

# include <list>
# include <queue>
# include <stack>
# include <vector>

namespace tree {

template <
	template <typename ...> typename Vector_ = std::vector,
	template <typename ...> typename List_ = std::list,
	template <typename ...> typename Stack_ = std::stack,
	template <typename ...> typename Queue_ = std::queue,
	template <typename> typename StackContainer_ = Vector_,
	template <typename> typename QueueContainer_ = List_
>
struct TypeConfig {
public:
	template <typename ... TS> using Vector = Vector_ <TS ...>;
	template <typename ... TS> using List = List_ <TS ...>;
	template <typename T> using Stack = Stack_ <T, StackContainer_ <T>>;
	template <typename T> using Queue = Queue_ <T, QueueContainer_ <T>>;

	// using TypeConfigCustom = TypeConfig <vector, list, stack, queue>;
	using TypeConfigStd = TypeConfig <std::vector, std::list, std::stack, std::queue>;
	// using TypeConfigMix1 = TypeConfig <vector, list, stack, queue, std::vector, std::list>;
	// using TypeConfigMix2 = TypeConfig <vector, list, std::stack, std::queue, vector, list>;
};

using TreeTypeConfig = TypeConfig<>::TypeConfigStd;

template <typename ... TS> using vector = TreeTypeConfig::Vector <TS ...>;
template <typename ... TS> using list = TreeTypeConfig::List <TS ...>;
template <typename T> using stack = TreeTypeConfig::Stack <T>;
template <typename T> using queue = TreeTypeConfig::Queue <T>;

} // end namespace tree


# endif // TREE_CONFIG_H
