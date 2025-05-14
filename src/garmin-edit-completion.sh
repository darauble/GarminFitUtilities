#!/bin/bash

# Complete the `garmin-edit` command with its subcommands.

_garmin_edit_completion() {
    local cur prev opts subopts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    # First level options: replace, set, show
    opts="replace set show"

    # Subcommand completions
    case "${prev}" in
        garmin-edit)
            COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
            return 0
            ;;
        replace)
            # Second level for 'replace' subcommand
            subopts="gpx product"
            COMPREPLY=( $(compgen -W "${subopts}" -- ${cur}) )
            return 0
            ;;
        set)
            # Second level for 'set' subcommand
            subopts="raw timestamp"
            COMPREPLY=( $(compgen -W "${subopts}" -- ${cur}) )
            return 0
            ;;
        show)
            # Second level for 'show' subcommand
            subopts="activities gpx help message product raw timestamp"
            case "${COMP_CWORD}" in
                3)
                    # Third level for 'show' options
                    case "${prev}" in
                        activities|gpx|help|message|product|raw|timestamp)
                            COMPREPLY=( $(compgen -W "${subopts}" -- ${cur}) )
                            return 0
                            ;;
                    esac
                    ;;
                *)
                    COMPREPLY=( $(compgen -W "${subopts}" -- ${cur}) )
                    return 0
                    ;;
            esac
            return 0
            ;;
        *)
            ;;
    esac
}

complete -F _garmin_edit_completion garmin-edit

