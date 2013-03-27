#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "nm-platform.h"
#include "nm-linux-platform.h"
#include "nm-fake-platform.h"

static const char *
type_to_string (NMLinkType type)
{
	switch (type) {
	case NM_LINK_TYPE_LOOPBACK:
		return "loopback";
	case NM_LINK_TYPE_ETHERNET:
		return "ethernet";
	case NM_LINK_TYPE_DUMMY:
		return "dummy";
	default:
		return "unknown-type";
	}
}

static void
dump_interface (NMPlatformLink *link)
{
	GArray *ip6_addresses;
	GArray *ip4_addresses;
	const NMPlatformIP6Address *ip6_address;
	const NMPlatformIP4Address *ip4_address;
	char addrstr[INET6_ADDRSTRLEN];
	int i;

	g_assert (link->up || !link->connected);

	printf ("%d: %s: %s", link->ifindex, link->name, type_to_string (link->type));
	if (link->up)
		printf (" %s", link->connected ? "CONNECTED" : "DISCONNECTED");
	else
		printf (" DOWN");
	if (!link->arp)
		printf (" noarp");
	printf ("\n");

	ip4_addresses = nm_platform_ip4_address_get_all (link->ifindex);
	ip6_addresses = nm_platform_ip6_address_get_all (link->ifindex);

	g_assert (ip4_addresses);
	g_assert (ip6_addresses);

	for (i = 0; i < ip4_addresses->len; i++) {
		ip4_address = &g_array_index (ip4_addresses, NMPlatformIP4Address, i);
		inet_ntop (AF_INET, &ip4_address->address, addrstr, sizeof (addrstr));
		printf ("    ip4-address %s/%d\n", addrstr, ip4_address->plen);
	}

	for (i = 0; i < ip6_addresses->len; i++) {
		ip6_address = &g_array_index (ip6_addresses, NMPlatformIP6Address, i);
		inet_ntop (AF_INET6, &ip6_address->address, addrstr, sizeof (addrstr));
		printf ("    ip6-address %s/%d\n", addrstr, ip6_address->plen);
	}

	g_array_unref (ip4_addresses);
	g_array_unref (ip6_addresses);
}

static void
dump_all (void)
{
	GArray *links = nm_platform_link_get_all ();
	int i;

	for (i = 0; i < links->len; i++)
		dump_interface (&g_array_index (links, NMPlatformLink, i));
}

int
main (int argc, char **argv)
{
	g_type_init ();

	g_assert (argc <= 2);
	if (argc > 1 && !g_strcmp0 (argv[1], "--fake"))
		nm_fake_platform_setup ();
	else
		nm_linux_platform_setup ();

	dump_all ();

	return EXIT_SUCCESS;
}
