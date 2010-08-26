//===========================================================================80>
/**
 * @file PopularizedResourceCollection.cc
 *
 * PopularizedResourceCollection.cc
 *
 * Created: Aug 26, 2010
 *
 * @todo Add GPL notice.
 *
 * The Zipf distribution functions are based off of code
 * provided in BYU CS 360's Winter 2010 semester.
 */

// from inet
#include "PopularizedResourceCollection.h"

// from omnetpp extension
#include <omnetppextension.h>

// from standard C++ libraries
#include <cmath> // for pow

//---

PopularizedResourceCollection::PopularizedResourceCollection()
{
	_next_rank = 0;
	_zipf_rank_offset = 0;
	_zipf_exponent = 0;
	_zipf_inverse_normalize_factor = 0;
	_zipf_normalize_factor = 0;
	_adding_resources = false;

	_resource_vector[0] = NULL;
}

PopularizedResourceCollection::~PopularizedResourceCollection()
{
	PopularizedResourceVector::iterator rv_itr = _resource_vector.begin();
	for ( ; rv_itr != _resource_vector.end(); rv_itr++)
	{
		deleteSafe((*rv_itr));
	}
}

bool PopularizedResourceCollection::addResource(PopularizableResource * resource)
{
	rank_t rank = nextRank();

	if (rank == RANK_NULL)
	{
		return false;
	}
	// else
	_adding_resources = true;
	updateInverseNormalizeFactor(rank);

	PopularizedResource * entry = new PopularizedResource(rank, resource);
	_resource_vector[rank] = entry;
	return true;
}

PopularizableResource * PopularizedResourceCollection::selectResource()
{
	// get a probability [0,1]
	double desired_popularity = uniform(0,1);

	return selectResource(desired_popularity);
}

PopularizableResource * PopularizedResourceCollection::selectResource(double desired_popularity)
{
	if(_resource_vector.empty() || desired_popularity < 0 || 1 < desired_popularity)
	{
		return NULL;
	}

	if (_adding_resources)
	{
		_adding_resources = false;
		updatePopularityOfAllResources();
	}

	rank_t current_rank = 0;
	double current_popularity = 0;

	do
	{
		current_rank++;

		ASSERT(current_rank < (int) _resource_vector.size());

		current_popularity += _resource_vector[current_rank]->popularity();

	} while (current_popularity < desired_popularity);

	return selectResource(current_rank);
}

PopularizableResource * PopularizedResourceCollection::selectResource(rank_t rank)
{
	if (rank < 1 || (int) _resource_vector.size() <= rank)
	{
		return NULL;
	}

	_resource_vector[rank]->select();

	return _resource_vector[rank]->resource();
}

rank_t PopularizedResourceCollection::nextRank()
{
	if (_next_rank == RANK_MAX)
	{
		return RANK_NULL;
	}
	// else
	return ++_next_rank; // notice pre-increment
}

void PopularizedResourceCollection::updateInverseNormalizeFactor(rank_t rank)
{
	_zipf_inverse_normalize_factor += ( 1.0 / std::pow( (rank+_zipf_rank_offset), _zipf_exponent) );
}

void PopularizedResourceCollection::updateNormalizeFactor()
{
	ASSERT(_zipf_inverse_normalize_factor != 0);

	_zipf_normalize_factor = 1.0 / _zipf_inverse_normalize_factor;
}

double PopularizedResourceCollection::popularityOf(rank_t rank)
{
	return (_zipf_normalize_factor /
			( std::pow(rank + _zipf_rank_offset, _zipf_exponent) ) );
}

void PopularizedResourceCollection::updatePopularityOfAllResources()
{
	if (_adding_resources)
	{
		return;
	}
	// else
	updateNormalizeFactor();

	PopularizedResourceVector::iterator rv_itr = _resource_vector.begin();
	for ( ; rv_itr != _resource_vector.end(); rv_itr++)
	{
		(*rv_itr)->setPopularity( popularityOf( (*rv_itr)->rank() ) );
	}
}



//---



PopularizedResource::PopularizedResource()
{
	setRank(RANK_NULL);
	setPopularity(0);
	resetTimesSelected();
	_resource = NULL;
}

PopularizedResource::PopularizedResource(rank_t rank, PopularizableResource * resource)
{
	setRank(rank);
	setPopularity(0);
	resetTimesSelected();

	_resource = NULL;
	setResource(resource);
}

PopularizedResource::~PopularizedResource()
{
	deleteSafe(_resource);
}

void PopularizedResource::setResource(PopularizableResource * resource)
{
	deleteSafe(_resource);
	_resource = resource;
}

void PopularizedResource::select()
{
	if (_times_selected != COUNTER_MAX)
	{
		_times_selected++;
	}
}

//---


