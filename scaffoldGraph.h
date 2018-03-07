/*
Copyright (C) 2014 Itoh Laboratory, Tokyo Institute of Technology

This file is part of Platanus.

Platanus is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Platanus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with Platanus; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef SCAFFOLD_GRAPH_H
#define SCAFFOLD_GRAPH_H
#define STATIC_VERSION

#include "seqlib.h"
#include "mapper.h"
#include <unordered_map>
#include <vector>


class ScaffoldGraph
{
private:
    /*
     * define inner classes using Scaffold class menber function
     */
    struct ScaffoldPart
    {
        long id;
        long start;
        long end;

        ScaffoldPart(): id(0), start(0), end(0) {}
        ScaffoldPart(long a, long b, long c): id(a), start(b), end(c) {}
        ScaffoldPart(const ScaffoldPart &) = default;
        ScaffoldPart &operator=(const ScaffoldPart &) = default;
        ~ScaffoldPart() = default;
    };

    struct GraphEdge
    {
        char direction;
        long end;
        long length;
        long numLink;
        std::vector<long> breakdown;

        GraphEdge(): direction(0), end(0), length(0), numLink(0), breakdown() {}
        GraphEdge(const GraphEdge &) = default;
        GraphEdge &operator=(const GraphEdge &) = default;
        ~GraphEdge() = default;

        bool isForward(void) const {return direction > 0; }

        bool operator<(const GraphEdge &a) const
        {
            if (direction != a.direction) 
                return direction < a.direction;
            else
                return end < a.end;
        }
    };

    struct GraphNode
    {
        bool isHomo;
        char state;
        long length;
        long numEdge;
        std::vector<GraphEdge> edge;
        long numContig;
        std::vector<ScaffoldPart> contig;

        GraphNode(): isHomo(false), state(0), length(0), numEdge(0), edge(), numContig(0), contig() {}
        GraphNode(const GraphNode &) = default;
        GraphNode &operator=(const GraphNode &) = default;
        ~GraphNode() = default;
    };

    struct GraphLink
    {
        long id1;
        long id2;
        long offset1;
        long offset2;
        long gap;

        GraphLink(): id1(0), id2(0), offset1(0), offset2(0), gap(0) {}
        ~GraphLink() = default;
        void clearValue(void)
        {
            id1 = 0;
            id2 = 0;
            offset1 = 0;
            offset2 = 0;
            gap = 0;
        }
        bool operator<(const GraphLink &a) const
        {
            if (id1 != a.id1)
                return (id1 - a.id1) < 0;
            else if (id2 != a.id2)
                return (id2 - a.id2) < 0;
            else
                return (gap - a.gap) < 0;
        }
    };

    struct GraphLinkPoolIndex
    {
        unsigned long index;
        long numLink;

        GraphLinkPoolIndex(): index(0), numLink(0) {}
        GraphLinkPoolIndex(unsigned long idx): index(idx), numLink(0) {}
        ~GraphLinkPoolIndex() = default;
    };

    struct GraphLinkPoolIndexGreater
    {
        bool operator() (const GraphLinkPoolIndex& link1, const GraphLinkPoolIndex& link2) const
        { return link1.numLink > link2.numLink; }
    };

    struct GraphLayout
    {
        long id;
        long start;
        long end;
        long distance;
        long numLink;

        GraphLayout(): id(0), start(0), end(0), distance(0), numLink(0) {}
        ~GraphLayout() = default;

        bool operator<(const GraphLayout &a) const
        {
            return (start - a.end - (a.start - end)) < 0;
        }
    };

    struct Overlap
    {
        int id1;
        int id2;
        int length;

        Overlap(): id1(0), id2(0), length(0) {}
        ~Overlap() = default;
    };
    // end definition inner classes

    static const unsigned TABLE_DIVID;
    static const double MAX_DIFF_RATE;
    static const double EDGE_EXPECTED_RATE_TH;
    static const double EDGE_EXPECTED_RATE_UPPER_TH;
    static const double CHECK_USING_LONGER_LIB_TH;
    static const unsigned SC_REP;
    static const unsigned SC_INC;
    static const unsigned SC_DEL;
    static const double MAX_HOMO_RATE;
    static const double MAX_HETERO_RATE;
    static const double MAX_OVERLAP_IDENTITY_DIFF;

    long seedLength;
    long minOverlap;
    long hashOverlap;
    long indexLength;
    long minLink;
    long tolerence;
    long minTolerenceFactor;
    long genomeSize;
    long numContig;
    long numNode;
    double averageCoverage;
    double bubbleThreshold;
    FILE* contigFP;
    FILE* bubbleFP;
    FILE* overlapFP;
    FILE* graphLinkFP;
    std::vector<platanus::SEQ> contig;
    std::vector<SeqLib> library;
    std::vector<unsigned short> coverage;
    std::vector<char> seqPool;
    std::vector<GraphNode> node;
    std::vector<long> numBubble;
    std::vector<platanus::Position> contigPositionInScaffold;
    std::vector<std::unordered_map<std::pair<int, int>, Overlap, platanus::PairHash, platanus::PairEqual> > overlapTable;

    void destroyGraph(void);
    long getOverlap(long id1, long id2);
    long getShortOverlap(long id1, long id2) const;
    long getScaffoldOverlap(long id1, long id2);
    bool checkDeleteEdge(const GraphEdge &edge1, const GraphEdge &edge2, const GraphNode &node1, const GraphNode &node2);
    long deleteErroneousEdge(const long numThread);
    void deleteEdges(std::vector<long> &ids);
    void remake(const long numNewNode, const long numContigPoolSize, FILE *scaffoldFP);
    double calcExpectedLink(const double link1, const double link2, const double g) const;
    double calcNodeCoverage(const GraphNode &node);
    unsigned long long crushBubble(const double bubbleThreshold, const double averageCoverage, const long numThread);
    void layoutNodes(GraphNode *newNode, std::vector<GraphLayout> &ret, std::vector<GraphLayout> &work);
    void layout2seq(const std::vector<GraphLayout> &lerfOverlap, const long startPoint, const long numLeftOverlap, std::vector<char> &ret);
    long alignScaffold(const std::vector<char> &scaffold1, const std::vector<char> &scaffold2, std::vector<long> &work, const long scoreThreshold) const;
    double layoutAverageCoverage(const std::vector<GraphLayout> &leftOverlap, const long startPoint, const long leftOverlapSize) const;
    long getSimilarOverlap(const long id1, const long id2);
    void calcLinkAndWriteGraphLinkFile(const std::vector<GraphLink>& links, const GraphLinkPoolIndex& index);
    void calcLinkAverageAndWriteGraphLinkFile(const std::vector<GraphLink>& links, const GraphLinkPoolIndex& index);
    long estimateGapSize(const std::vector<GraphLink>& links, const unsigned index, const unsigned size) const;
    long estimateGapSizeAverage(const std::vector<GraphLink>& links, const unsigned index, const unsigned size) const;
    long calcNumPossiblePosition(const long length1, const long length2, const long distance, const long insSize) const;
    long calcNumPossiblePositionNode(const GraphNode& node1, const GraphNode& node2, const long distance, const long insSize) const;
    long calcNumPossiblePositionNodeTemp(const long node1Start, const long node1End, const long node1Length, const GraphNode& node2, const long distance, const long insSize) const;
    double calcExpectedLinkNode(const GraphNode& node1, const GraphNode& node2, const long distance) const;
    double calcExpectedLinkNodeTemp(const long node1Start, const long node1End, const long node1Length, const GraphNode& node2, const long distance) const;

    unsigned decideTableID(const unsigned long long key)
    {
        static const unsigned long long ander = TABLE_DIVID - 1;
        return key & ander;
    }

public:
    ScaffoldGraph();
    ScaffoldGraph(const ScaffoldGraph&) = delete;
    ScaffoldGraph& operator=(const ScaffoldGraph&) = delete;

    void saveOverlap(const Mapper &map, const long hashOverlapValue, const long cutoffLengthi, const long numThread);
    void makeGraph(const long numThread);
    void calcLink(const long numThread);
    void detectRepeat(const double averageCoverage);
    void deleteRepeatEdge(void);
    void deleteErroneousEdgeIterative(const long numThread);
    void split(void);
    void makeScaffold(void);
    long estimateLink(void);
    void crushBubbleIterative(const double bubleThreshold, const double averageCoverage, const long numThread);
    unsigned long long crushHeteroBubble(const double averageCoverage);
    void cutAndPrintSeq(const long minSeqLength, const std::string &outFilename, const std::string &componentFilename);
    void initScaffolding(std::vector<unsigned short> &cov, Mapper &mapper, const double ave, const double bubble);
    void countBubble(const platanus::Contig &bubble, const std::vector<platanus::Position> &bubblePosition);
    void classifyNode(void);
    long deleteHeteroEdge(void);
    void removeHeteroOverlap(void);
    void splitLowCoverageLink(const std::vector<std::vector<unsigned> >& numErroneousPair, const std::vector<std::vector<unsigned> >& numSpanningPair, const std::vector<std::vector<double> >& sumExpectedLink, std::unordered_map<std::pair<int, int>, bool, platanus::PairHash, platanus::PairEqual> &errorLink, const long minLink, const long numThread);
    void countPairsSpanningGap(std::vector<std::vector<unsigned> >& numSpanningPair, const long numThread);
    void splitLowCoverageLinkAndDeleteErrorneousMappedPair(std::vector<std::vector<SeqLib> > &libraryMT, const long minLink, const long numThread);
    void countPairsLinkingInsideContigs(std::vector<std::vector<unsigned> >& numPair, const long numThread);
    void printScaffoldBubble(const std::string &outFilename);
    void insertSizeDistribution(std::vector<SeqLib>& library, std::vector<long>& distribution, const long numThread);
    void scaffoldLengthList(std::vector<long>& list);

    long getTolerence(void) const { return tolerence; }
    long getNumNode(void) const { return numNode; }
    void setSeedLength(const long len) { seedLength = len; }
    void setMinOverlap(const long olp) { minOverlap = olp; }
    void setSeqLib(const std::vector<SeqLib> &lib) { library = lib; }
    void setTolerence(const long tol) { tolerence = tol; }
    void setMinTolerenceFactor(const long fac) { minTolerenceFactor = fac; }
    void setMinLink(const unsigned long num) { minLink = num; }
};


//////////////////////////////////////////////////////////////////////////////////////
// calc expected number of links
//////////////////////////////////////////////////////////////////////////////////////
inline double ScaffoldGraph::calcExpectedLink(const double link1, const double link2, const double g) const
{
    const double averageIns = (double)library[0].getAverageInsSize();
    const double sdIns = (double)library[0].getSDInsSize();
#ifdef STATIC_VERSION
    const double average = (double)library[0].getAverageLength();
#else
    double average = (double)library[0].getAverageLength() / 2;
#endif
    const double coverage = library[0].getAverageCoverage();
    double numLink = 0;

    numLink += (link1 + g - averageIns + link2) * erf((link1 + g - averageIns + link2) / (M_SQRT2 * sdIns));
    numLink += (M_SQRT2 * sdIns / sqrt(M_PI)) * exp(-pow(((link1 + g - averageIns + link2) / (M_SQRT2 * sdIns)), 2.0));
    numLink -= (average + g - averageIns + link2) * erf((average + g - averageIns + link2) / (M_SQRT2 * sdIns));
    numLink -= (M_SQRT2 * sdIns / sqrt(M_PI)) * exp(-pow(((average + g - averageIns + link2) / (M_SQRT2 * sdIns)), 2.0));

    numLink -= (link1 + g - averageIns + average) * erf((link1 + g - averageIns + average) / (M_SQRT2 * sdIns));
    numLink -= (M_SQRT2 * sdIns / sqrt(M_PI)) * exp(-pow(((link1 + g - averageIns + average) / (M_SQRT2 * sdIns)), 2.0));
    numLink += (average + g - averageIns + average) * erf((average + g - averageIns + average) / (M_SQRT2 * sdIns));
    numLink += (M_SQRT2 * sdIns / sqrt(M_PI)) * exp(-pow(((average + g - averageIns + average) / (M_SQRT2 * sdIns)), 2.0));

#ifndef STATIC_VERSION
    average *= 2;
#endif
    numLink *= coverage / (4.0 * average);

    return numLink;

}


//////////////////////////////////////////////////////////////////////////////////////
// calc expected number of links between two nodes
//////////////////////////////////////////////////////////////////////////////////////
inline double ScaffoldGraph::calcExpectedLinkNode(const GraphNode& node1, const GraphNode& node2, const long distance) const
{
    if (node1.contig.empty() || node2.contig.empty()) return 1.0;

    double expected = 0;
    long node1Start = node1.contig[0].start;
    long node1End   = node1.contig[0].end;
    for (unsigned idx1 = 1; idx1 < node1.contig.size(); ++idx1) {
        if (node1.contig[idx1].start <= node1End) {
            node1End = node1.contig[idx1].end;
            continue;
        }
        expected += calcExpectedLinkNodeTemp(node1Start, node1End, node1.length, node2, distance);
        node1Start = node1.contig[idx1].start;
        node1End   = node1.contig[idx1].end;
    }
    return expected + calcExpectedLinkNodeTemp(node1Start, node1End, node1.length, node2, distance);
}


//////////////////////////////////////////////////////////////////////////////////////
// calc expected number of links between two nodes (utility)
//////////////////////////////////////////////////////////////////////////////////////
inline double ScaffoldGraph::calcExpectedLinkNodeTemp(const long node1Start, const long node1End, const long node1Length, const GraphNode& node2, const long distance) const
{
    double expected = 0;
    long node2Start = node2.contig[0].start;
    long node2End   = node2.contig[0].end;
    for (unsigned idx2 = 1; idx2 < node2.contig.size(); ++idx2) {
        if (node2.contig[idx2].start < node2End) {
            node2End = node2.contig[idx2].end;
            continue;
        }
        expected += calcExpectedLink(
                node1End - node1Start + 1, node2End - node1Start + 1,
                distance + node1Length - node1End + node2Start);
        node2Start = node2.contig[idx2].start;
        node2End   = node2.contig[idx2].end;
    }
    expected += calcExpectedLink(
            node1End - node1Start + 1, node2End - node1Start + 1,
            distance + node1Length - node1End + node2Start);
    return expected;
}


//////////////////////////////////////////////////////////////////////////////////////
// calc possible number of positions that reads can be mapped
//////////////////////////////////////////////////////////////////////////////////////
inline long ScaffoldGraph::calcNumPossiblePosition(const long length1, const long length2, const long distance, const long insSize) const
{
    const long minNodeLength = std::min(length1, length2);
    const long totalNodeLength = length1 + length2;
    const long readLength = library[0].getAverageLength() / 2;

    long way = std::max(0l, insSize - (distance > 0 ? distance : 0) - readLength * 2 + 1);
    way = std::min(way, std::max(0l, minNodeLength + readLength + 1));
    way = std::min(way, std::max(0l, totalNodeLength + distance - insSize + 1));
    return way;
}


//////////////////////////////////////////////////////////////////////////////////////
// calc possible number of positions that reads can be mapped between two nodes
//////////////////////////////////////////////////////////////////////////////////////
inline long ScaffoldGraph::calcNumPossiblePositionNode(const GraphNode& node1, const GraphNode& node2, const long distance, const long insSize) const
{
    if (node1.contig.empty() || node2.contig.empty()) return 0;

    long way = 0;
    long node1Start = node1.contig[0].start;
    long node1End   = node1.contig[0].end;
    for (unsigned idx1 = 1; idx1 < node1.contig.size(); ++idx1) {
        if (node1.contig[idx1].start <= node1End) {
            node1End = node1.contig[idx1].end;
            continue;
        }
        way += calcNumPossiblePositionNodeTemp(node1Start, node1End, node1.length, node2, distance, insSize);
        node1Start = node1.contig[idx1].start;
        node1End   = node1.contig[idx1].end;
    }
    return way + calcNumPossiblePositionNodeTemp(node1Start, node1End, node1.length, node2, distance, insSize);
}


//////////////////////////////////////////////////////////////////////////////////////
// calc possible number of positions that reads can be mapped between two nodes (utility)
//////////////////////////////////////////////////////////////////////////////////////
inline long ScaffoldGraph::calcNumPossiblePositionNodeTemp(const long node1Start, const long node1End, const long node1Length, const GraphNode& node2, const long distance, const long insSize) const
{
    long way = 0;
    long node2Start = node2.contig[0].start;
    long node2End   = node2.contig[0].end;
    for (unsigned idx2 = 1; idx2 < node2.contig.size(); ++idx2) {
        if (node2.contig[idx2].start < node2End) {
            node2End = node2.contig[idx2].end;
            continue;
        }
        way += calcNumPossiblePosition(
                node1End - node1Start + 1, node2End - node2Start + 1,
                distance + node1Length - node1End + node2Start, insSize);
        node2Start = node2.contig[idx2].start;
        node2End   = node2.contig[idx2].end;
    }
    way += calcNumPossiblePosition(
            node1End - node1Start + 1, node2End - node2Start + 1,
            distance + node1Length - node1End + node2Start, insSize);
    return way;
}




#endif
