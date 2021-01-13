#pragma once

#include <Core/EngineCore.h>
#include <Core/ClassTemplates.h>
#include <Core/BaseTypes.h>


KRS_BEGIN_NAMESPACE(Kairos)

class RenderPass {
public:
	KRS_CLASS_DEFAULT(RenderPass);

	void Evaluate();
	void Construct();

	void AddInputDependency(std::string name);
};





class RenderGraph {
public:



	KRS_CLASS_NON_COPYABLE_AND_MOVABLE(RenderGraph);


	void Build() {
		BuildAdjacencyLists();
		TopologicalSort();
	}
private:
	using RenderPassList = std::vector<RenderPass>;
	using AdjacencyMatrix = std::vector<std::vector<uint64_t>>;



	RenderPassList m_Nodes;
	AdjacencyMatrix m_AdjacencyMatrix;

	void BuildAdjacencyLists();
	void TopologicalSort();
};



KRS_END_NAMESPACE