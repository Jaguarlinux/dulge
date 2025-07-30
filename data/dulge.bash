_dulge_parse_help() {
	local IFS line word

	$1 --help 2>&1 | while IFS=$'\n' read -r line; do
		[[ $line == *([ $'\t'])-* ]] || continue

		IFS=$' \t,='
		for word in $line; do
			[[ $word == -* ]] || continue
			printf -- '%s\n' $word
		done
	done | sort | uniq
}

_dulge_all_packages() {
	dulge-query -Rs "$1*" | sed 's/^... \([^ ]*\)-.* .*/\1/'
}

_dulge_installed_packages() {
	dulge-query -l | sed 's/^.. \([^ ]*\)-.* .*/\1/'
}

_dulge_all_reply() {
	COMPREPLY=( $( compgen -W '$(_dulge_all_packages "$1")' -- "$1") )
}

_dulge_installed_reply() {
	COMPREPLY=( $( compgen -W '$(_dulge_installed_packages)' -- "$1") )
}

_dulge_complete() {
	local cur prev words cword

	_init_completion || return

	if [[ "$cur" == -* ]]; then
		COMPREPLY=( $( compgen -W '$( _dulge_parse_help "$1" )' -- "$cur") )
		return
	fi

	local common='C|-config|r|-rootdir'
	local morecommon="$common|c|-cachedir"

	local modes='auto manual hold unhold'
	local props='architecture
		archive-compression-type
		automatic-install
		build-options
		conf_files
		conflicts
		filename-sha256
		filename-size
		homepage
		install-date
		install-msg
		install-script
		installed_size
		license
		maintainer
		metafile-sha256
		packaged-with
		pkgver
		preserve
		provides
		remove-msg
		remove-script
		replaces
		repository
		shlib-provides
		shlib-requires
		short_desc
		source-revisions
		state'

	case $1 in
		dulge-dgraph)
			if [[ $prev != -@(c|o|r) ]]; then
				_dulge_installed_reply $cur
				return
			fi
			;;
		dulge-install)
			if [[ $prev != -@($morecommon) ]]; then
				_dulge_all_reply $cur
				return
			fi
			;;
		dulge-pkgdb)
			if [[ $prev == -@(m|-mode) ]]; then
				COMPREPLY=( $( compgen -W "$modes" -- "$cur") )
				return
			fi
			if [[ $prev != -@($common) ]]; then
				_dulge_installed_reply $cur
				return
			fi
			;;
		dulge-query)
			if [[ $prev == -@(p|-property) ]]; then
				COMPREPLY=( $( compgen -W "$props" -- "$cur") )
				return
			fi
			if [[ $prev != -@($morecommon|o|-ownedby) ]]; then
				local w
				for w in "${words[@]}"; do
					if [[ "$w" == -@(R|-repository) ]]; then
						_dulge_all_reply $cur
						return
					fi
				done
				_dulge_installed_reply $cur
				return
			fi
			;;
		dulge-reconfigure)
			if [[ $prev != -@($common) ]]; then
				_dulge_installed_reply $cur
				return
			fi
			;;
		dulge-remove)
			if [[ $prev != -@($morecommon) ]]; then
				_dulge_installed_reply $cur
				return
			fi
			;;
	esac

	_filedir
}

complete -F _dulge_complete dulge-checkvers dulge-create dulge-dgraph dulge-install \
	dulge-pkgdb dulge-query dulge-reconfigure dulge-remove dulge-rindex
