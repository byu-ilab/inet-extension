//===========================================================================80>
/**
 * @file PopularizedResourceCollection.h
 *
 * PopularizedResourceCollection class declarations
 *
 * Created: Aug 26, 2010
 *
 * @todo Add GPL notice.
 */

#ifndef _POPULARIZEDRESOURCECOLLECTION_H_
#define _POPULARIZEDRESOURCECOLLECTION_H_

// from standard C++ libraries
#include <vector>
#include <iostream>

// from inet
#include "InternetTypeDefsWithStandardTypes.h"

class PopularizableResource;
class PopularizedResource;

typedef std::vector<PopularizedResource *> PopularizedResourceVector;
typedef int rank_t;

#define RANK_NULL ((rank_t) -1)
#define RANK_MAX  ((rank_t) 2147483647)

class PopularizedResourceCollection
{
public:
	PopularizedResourceCollection();
	virtual ~PopularizedResourceCollection();

	virtual bool addResource(PopularizableResource * resource);
	virtual PopularizableResource * selectResource();
	virtual PopularizableResource * selectResource(double popularity);
	virtual PopularizableResource * selectResource(int rank);

	// could be encapsulated in a stratecy object?
	virtual void setRankOffset(double rank_offset);
	virtual void setExponent(double exponent) { _zipf_exponent = exponent; }

	double rankOffset() const { return _zipf_rank_offset; }
	double exponent() const { return _zipf_exponent; }
	double normalizeFactor() const { return _zipf_normalize_factor; }

	virtual void writeSummary(std::ostream & os);

protected:
	rank_t nextRank();
	void updateInverseNormalizeFactor(rank_t rank);
	void updateNormalizeFactor();
	double popularityOf(rank_t rank);
	void updatePopularityOfAllResources();

private:
	rank_t _next_rank;
	PopularizedResourceVector _resource_vector;
	bool _adding_resources;

	counter_t _resource_selections;

	// could be encapsulated in a strategy object?
	double _zipf_normalize_factor;
	double _zipf_inverse_normalize_factor;
	double _zipf_rank_offset;
	double _zipf_exponent;
};

/**
 * Empty class from which resources containers can be created and then
 * stored in the PopularizedResourceCollection.
 */
class PopularizableResource
{
public:
	virtual ~PopularizableResource() {}

	virtual str_t toString() =0;
};

/**
 * Popularity determinates of a resource.
 */
class PopularizedResource
{
public:
	PopularizedResource();
	PopularizedResource(rank_t rank, PopularizableResource * resource);
	virtual ~PopularizedResource();

	virtual void setRank(rank_t rank) { _rank = rank; }
	virtual void setPopularity(double popularity) { _popularity = popularity; }
	virtual void setResource(PopularizableResource * resource);

	virtual void select();
	virtual void resetTimesSelected() { _times_selected = 0; }

	rank_t rank() const { return _rank; }
	double popularity() const { return _popularity; }
	counter_t timesSelected() const { return _times_selected; }
	PopularizableResource * resource() const { return _resource; }

private:
	rank_t _rank;
	double _popularity;
	counter_t _times_selected;
	PopularizableResource * _resource;
};

#endif /* _POPULARIZEDRESOURCECOLLECTION_H_ */
