/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsobane <hsobane@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 17:42:18 by zelhajou          #+#    #+#             */
/*   Updated: 2024/07/10 03:06:22 by hsobane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Sockets.hpp"

int main(int argc, char *argv[]) {
	(void)argc;
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	try {
		Parser parser(argv[1]); // Parse the config file
		Sockets s(parser); // Create the Sockets for all servers
		s.run(); // Start the servers using kqueue
	}
	catch (std::exception &e) {
		// If none of the servers could be started, the program should exit with a message to stderr
		std::cerr << e.what() << std::endl;
	}
    return 0;
}