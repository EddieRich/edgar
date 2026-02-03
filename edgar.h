#ifndef __EDGAR_DEFINED__
#define __EDGAR_DEFINED__

/*
	curl -s -A ehandrich@gmail.com "https://data.sec.gov/api/xbrl/companyfacts/CIK$cid.json" | jq . > "$factfile"

	shares_outstanding=$(jq -r '.facts.dei.EntityCommonStockSharesOutstanding.units.shares.[-1].val' "$factfile")

	local entries=(
		"NetCashProvidedByUsedInOperatingActivities"
		"NetCashProvidedByUsedInOperatingActivitiesContinuingOperations"
	)

	local arr="[]"
	local entry
	for entry in $"${entries[@]}"; do
		# get only the 10-K facts
		facts=$(jq --arg e "$entry" '.facts."us-gaap".[$e].units.USD | map(select(.form == "10-K"))' $factfile)
		if [ -n "$facts" ]; then
			arr=$(jq -n --argjson arr1 "$arr" --argjson arr2 "$facts" '$arr1 + $arr2')
		fi
	done
*/

void get_facts(char* ticker);

#endif // __EDGAR_DEFINED__
