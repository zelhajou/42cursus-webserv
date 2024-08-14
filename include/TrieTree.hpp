#ifndef __TRIETREE_HPP__
# define __TRIETREE_HPP__

# include <iostream>
# include <vector>
# include <string>
# include <iomanip>
# include <algorithm>
# include "ConfigStructures.hpp"

struct LocationNode
{
	std::string					name;
	LocationConfig*				location;
	int							lvl;
	std::vector<LocationNode*>	children;
};

bool			insert(std::vector<LocationNode *>& root, LocationNode* node, int (*cmp)(std::string, std::string));
LocationConfig*	search(std::vector<LocationNode *>& root, std::string name, int (*cmp)(std::string, std::string));
void			print(std::vector<LocationNode *>& root);
void			setlvl(std::vector<LocationNode *>& root);
int				cmp (std::string a, std::string b);
void			freeTrie(std::vector<LocationNode *>& root);



#endif
