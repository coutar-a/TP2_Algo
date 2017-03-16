//
// Created by Mario Marchand on 16-12-30.
//

#include "ReseauGTFS.h"
#include <sys/time.h>

using namespace std;

//détermine le temps d'exécution (en microseconde) entre tv2 et tv2
long tempsExecution(const timeval &tv1, const timeval &tv2)
{
    const long unMillion = 1000000;
    long dt_usec = tv2.tv_usec - tv1.tv_usec;
    long dt_sec = tv2.tv_sec - tv1.tv_sec;
    long dtms = unMillion * dt_sec + dt_usec;
    if (dtms < 0) throw logic_error("ReaseauGTFS::tempsExecution(): dtms doit être non négatif");
    return dtms;
}

size_t ReseauGTFS::getNbArcsOrigineVersStations() const
{
    return m_nbArcsOrigineVersStations;
}

size_t ReseauGTFS::getNbArcsStationsVersDestination() const
{
    return m_nbArcsStationsVersDestination;
}

double ReseauGTFS::getDistMaxMarche() const
{
    return distanceMaxMarche;
}

//! \brief construit le réseau GTFS à partir des données GTFS
//! \param[in] Un objet DonneesGTFS
//! \throws logic_error si une incohérence est détecté lors de la construction du graphe
//! \post constuit un réseau GTFS représenté par un graphe orienté pondéré avec poids non négatifs
//! \post initialise la variable m_origine_dest_ajoute à false car les points origine et destination ne font pas parti du graphe
//! \post insère les données requises dans m_arretDuSommet et m_sommetDeArret et construit le graphe m_leGraphe
ReseauGTFS::ReseauGTFS(const DonneesGTFS &p_gtfs)
: m_leGraphe(p_gtfs.getNbArrets()), m_origine_dest_ajoute(false)
{

    m_nbArcsStationsVersDestination = 0;
    m_nbArcsOrigineVersStations = 0;
    size_t ctr = 0;
    // m_arretDuSommet.resize(p_gtfs.getNbArrets());
    // for (auto elem : m_arretDuSommet) {elem = nullptr;}


    //ajout des arcs dus aux voyages et mise à jour de m_sommetDeArret ey m_arretDuSommet




    auto voyages = p_gtfs.getVoyages();

    for (auto it = voyages.begin() ; it != voyages.end() ; ++it) {
        std::set<Arret::Ptr, Voyage::compArret> arrets = it->second.getArrets();


        for (auto it2 = arrets.begin() ; it2 != arrets.end() ; ++it2) {

            if (it2 != arrets.begin()) {
                Arret::Ptr currentStop = *it2;
                auto prevIterator = std::prev(it2, 1);
                Arret::Ptr prevStop = *(prevIterator);
                int weight = currentStop->getHeureArrivee() - prevStop->getHeureArrivee();

                //std::cout << "first phase " <<weight << std::endl;
                if (weight < 0) {
                    throw std::logic_error("ReseauGTFS::ReseauGTFS() : Negative weight");
                }


            //we add the arc to the graph

                m_leGraphe.ajouterArc(m_sommetDeArret[prevStop], m_sommetDeArret[currentStop], weight);

                // m_leGraphe.ajouterArc(ctr, ctr + 1, weight);

                // if (m_arretDuSommet[ctr] == nullptr) {
                //     m_arretDuSommet[ctr] = prevStop;
                //     m_sommetDeArret.insert({prevStop, ctr});
                // }
                // m_arretDuSommet[ctr] = currentStop;
                // m_sommetDeArret.insert({currentStop, ctr});
                // if (m_arretDuSommet[ctr] == nullptr) {
                //     m_arretDuSommet[ctr] = prevStop;
                //     m_sommetDeArret.insert({prevStop, ctr});
                // }
                // ctr++;
                // m_arretDuSommet[ctr] = currentStop;
                // m_sommetDeArret.insert({currentStop, ctr});

                //  m_leGraphe.ajouterArc(ctr - 1, ctr, weight);

                //  if (m_arretDuSommet[ctr - 1] == nullptr) {
                //     m_arretDuSommet[ctr - 1] = prevStop;
                //     m_sommetDeArret.insert({prevStop, ctr - 1});
                // }
                // m_arretDuSommet[ctr] = currentStop;
                // m_sommetDeArret.insert({currentStop, ctr});
            }
            m_arretDuSommet.push_back(*it2);
            m_sommetDeArret.insert({*it2, (m_arretDuSommet.size() + 1)});
            //ctr++;

        }

    }

    std::cout << "phase 1 end " <<ctr << std::endl;
    std::cout << m_sommetDeArret.size() << std::endl;
    std::cout << m_leGraphe.getNbSommets() << std::endl;
    //ajout des arcs dus aux attentes à chaque station

    auto stationMap = p_gtfs.getStations();

    for (auto stationPair : stationMap) {

        auto stationStops = stationPair.second.getArrets();

        for (auto it = stationStops.begin() ; it != stationStops.end() ; ++it) {

          //  std::cout << it->first << std::endl;
            if (it != stationStops.begin()) {
                Arret::Ptr currentStop = it->second;
                auto prevIterator = std::prev(it, 1);
                Arret::Ptr prevStop = prevIterator->second;
                int weight = currentStop->getHeureArrivee() - prevStop->getHeureArrivee();

                if (weight < 0) {
                    throw std::logic_error("ReseauGTFS::ReseauGTFS() : Negative weight");
                }
                //std::cout << "second phase "<< weight << std::endl;
                //m_leGraphe.ajouterArc(numSeqPrev, numSeqArret, weight);
                //std::cout << m_sommetDeArret[prevStop] << " " <<  m_sommetDeArret[currentStop] << std::endl;
                m_leGraphe.ajouterArc(m_sommetDeArret[prevStop], m_sommetDeArret[currentStop], weight);
            }
        }

    }

    //ajouts des arcs dus aux transferts entre stations

    auto transferts = p_gtfs.getTransferts();

    for (auto instance : transferts) {

        unsigned int departure = std::get<0>(instance);
        unsigned int destination = std::get<1>(instance);
        unsigned int travelTime = std::get<2>(instance);
        Station origin = stationMap[departure];
        Station endPoint = stationMap[destination];

        auto startStops = origin.getArrets();
        auto destStops = endPoint.getArrets();

        for (auto stop : startStops) {

            Heure ETA = stop.second->getHeureDepart();
            ETA.add_secondes(travelTime);
            auto closestCandidate = destStops.lower_bound(ETA);

            if (closestCandidate != destStops.end()) {

                int weight = ((*closestCandidate).second->getHeureDepart() - stop.second->getHeureDepart()) + travelTime;
                if (weight < 0) {
                    throw std::logic_error("ReseauGTFS::ReseauGTFS() : Negative weight");
                }

               //std::cout << "third phase " << weight << std::endl;
                m_leGraphe.ajouterArc(m_sommetDeArret[stop.second], m_sommetDeArret[(*closestCandidate).second], weight);

            }
        }

    }

    m_origine_dest_ajoute = false;
}

//! \brief ajoute des arcs au réseau GTFS à partir des données GTFS
//! \brief Il s'agit des arcs allant du point origine vers une station si celle-ci est accessible à pieds et des arcs allant d'une station vers le point destination
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \param[in] p_pointOrigine: les coordonnées GPS du point origine
//! \param[in] p_pointDestination: les coordonnées GPS du point destination
//! \throws logic_error si une incohérence est détecté lors de la construction du graphe
//! \post constuit un réseau GTFS représenté par un graphe orienté pondéré avec poids non négatifs
//! \post assigne la variable m_origine_dest_ajoute à true (car les points orignine et destination font parti du graphe)
//! \post insère dans m_sommetsVersDestination les numéros de sommets connctés au point destination
void ReseauGTFS::ajouterArcsOrigineDestination(const DonneesGTFS &p_gtfs, const Coordonnees &p_pointOrigine,
   const Coordonnees &p_pointDestination)
{

    //values are uninmportant
    Arret::Ptr origine(new Arret(stationIdOrigine, Heure(0,0,0), Heure(0,0,0), 0, "42"));
    Arret::Ptr destination(new Arret(stationIdDestination, Heure(0,0,0), Heure(0,0,0), 99999, "42"));

    //index of the last element in m_arretDuSommet
    Arret::Ptr lastElement = m_arretDuSommet.back();
    size_t index = 0;
    for (auto it = m_arretDuSommet.begin() ; *it != lastElement ; ++it) {
        index++;
    }

    std::cout << index << std::endl;
    std::cout << p_gtfs.getNbArrets() << std::endl;
    std::cout << m_leGraphe.getNbSommets() << std::endl;
    // std::cout << ((m_arretDuSommet[index] == lastElement) ? "true" : "false") << std::endl;
    //origin info

    m_leGraphe.resize(p_gtfs.getNbArrets() + 2);
    ++index;
    m_arretDuSommet[index] = origine;
    m_sommetDeArret.insert({origine, index});
    m_sommetOrigine = index;
    m_origine_dest_ajoute = true;

    //destination info
    ++index;
    m_arretDuSommet[index] = destination;
    m_sommetDeArret.insert({destination, index});
    m_sommetDestination = index;

    //ajout des arcs à pieds entre le point source et les arrets des stations atteignables

    auto stationMap = p_gtfs.getStations();

    for (auto stationPair : stationMap) {

        Coordonnees stationCoords = stationPair.second.getCoords();
        double distance = stationCoords - p_pointOrigine;

        if (distance <= distanceMaxMarche) {

           // std::cout << distance << " " << distanceMaxMarche << std::endl;
            double travelTime = (distance / vitesseDeMarche) * 3600;
            auto stationStops = stationPair.second.getArrets();
            Heure startingHour = p_gtfs.getTempsDebut();
            startingHour.add_secondes(travelTime);
            auto closestCandidate = stationStops.lower_bound(startingHour);

         //   std::cout << stationPair.second.getDescription() << std::endl;

            if (closestCandidate != stationStops.end()) {
                //std::cout << (*closestCandidate).second->getStationId() << std::endl;

                //if ((*closestCandidate).second->getStationId() == 5036) {std::cout << distance << std::endl;}
                //int weight = ((*closestCandidate).second->getHeureDepart() - p_gtfs.getTempsDebut()) + travelTime;
                int weight = ((*closestCandidate).second->getHeureDepart() - startingHour);
                //int weight = ((*closestCandidate).second->getHeureDepart() - p_gtfs.getTempsDebut());


                if (weight < 0) {
                    throw std::logic_error("ReseauGTFS::ajouterArcsOrigineDestination() : Negative weight");
                }
                m_leGraphe.ajouterArc(m_sommetOrigine, m_sommetDeArret[(*closestCandidate).second], weight);
                ++m_nbArcsOrigineVersStations;
            }
        }
    }


    //ajout des arcs à pieds des arrêts de certaine stations vers l'arret point destination

    //auto stationMap = p_gtfs.getStations();

    for (auto stationPair : stationMap) {

        Coordonnees stationCoords = stationPair.second.getCoords();
        double distance = stationCoords - p_pointDestination;

       // std::cout << stationPair.second.getDescription() << std::endl;
        if (distance <= distanceMaxMarche) {

        //std::cout << distance << " " << distanceMaxMarche << std::endl;
            double travelTime = (distance / vitesseDeMarche) * 3600;
            auto stationStops = stationPair.second.getArrets();

            for (auto stop : stationStops) {

                //std::cout << stop.second->getStationId() << std::endl;
                 if (stop.second->getStationId() == 5036) {std::cout << distance << std::endl;}
                int weight = travelTime;

                m_leGraphe.ajouterArc(m_sommetDeArret[(stop).second], m_sommetDestination, weight);
                ++m_nbArcsStationsVersDestination;
            }
        }
    }

}

//! \brief Remet ReseauGTFS dans l'était qu'il était avant l'exécution de ReseauGTFS::ajouterArcsOrigineDestination()
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \throws logic_error si une incohérence est détecté lors de la modification du graphe
//! \post Enlève de ReaseauGTFS tous les arcs allant du point source vers un arrêt de station et ceux allant d'un arrêt de station vers la destination
//! \post assigne la variable m_origine_dest_ajoute à false (les points orignine et destination sont enlevés du graphe)
//! \post enlève les données de m_sommetsVersDestination
void ReseauGTFS::enleverArcsOrigineDestination()
{

}


//! \brief Trouve le plus court chemin menant du point d'origine au point destination préalablement choisis
//! \brief Permet également d'affichier l'itinéraire du voyage et retourne le temps d'exécution de l'algorithme de plus court chemin utilisé
//! \param[in] p_afficherItineraire: true si on désire afficher l'itinéraire et false autrement
//! \param[out] p_tempsExecution: le temps d'exécution de l'algorithme de plus court chemin utilisé
//! \throws logic_error si un problème survient durant l'exécution de la méthode
void ReseauGTFS::itineraire(const DonneesGTFS &p_gtfs, bool p_afficherItineraire, long &p_tempsExecution) const
{
    if (!m_origine_dest_ajoute)
        throw logic_error(
            "ReseauGTFS::afficherItineraire(): il faut ajouter un point origine et un point destination avant d'obtenir un itinéraire");

    vector<size_t> chemin;

    timeval tv1;
    timeval tv2;
    if (gettimeofday(&tv1, 0) != 0)
        throw logic_error("ReseauGTFS::afficherItineraire(): gettimeofday() a échoué pour tv1");
    unsigned int tempsDuTrajet = m_leGraphe.plusCourtChemin(m_sommetOrigine, m_sommetDestination, chemin);
    if (gettimeofday(&tv2, 0) != 0)
        throw logic_error("ReseauGTFS::afficherItineraire(): gettimeofday() a échoué pour tv2");
    p_tempsExecution = tempsExecution(tv1, tv2);

    if (tempsDuTrajet == numeric_limits<unsigned int>::max())
    {
        if (p_afficherItineraire)
            cout << "La destination n'est pas atteignable de l'orignine durant cet intervalle de temps" << endl;
        return;
    }

    if (tempsDuTrajet == 0)
    {
        if (p_afficherItineraire) cout << "Vous êtes déjà situé à la destination demandée" << endl;
        return;
    }

    //un chemin non trivial a été trouvé
    if (chemin.size() <= 2)
        throw logic_error("ReseauGTFS::afficherItineraire(): un chemin non trivial doit contenir au moins 3 sommets");
    if (m_arretDuSommet[chemin[0]]->getStationId() != stationIdOrigine)
        throw logic_error("ReseauGTFS::afficherItineraire(): le premier noeud du chemin doit être le point origine");
    if (m_arretDuSommet[chemin[chemin.size() - 1]]->getStationId() != stationIdDestination)
        throw logic_error(
            "ReseauGTFS::afficherItineraire(): le dernier noeud du chemin doit être le point destination");

    if (p_afficherItineraire)
    {
        std::cout << std::endl;
        std::cout << "=====================" << std::endl;
        std::cout << "     ITINÉRAIRE      " << std::endl;
        std::cout << "=====================" << std::endl;
        std::cout << std::endl;
    }

    if (p_afficherItineraire) cout << "Heure de départ du point d'origine: "  << p_gtfs.getTempsDebut() << endl;
    Arret::Ptr ptr_a = m_arretDuSommet.at(chemin[0]);
    Arret::Ptr ptr_b = m_arretDuSommet.at(chemin[1]);
    if (p_afficherItineraire)
        cout << "Rendez vous à la station " << p_gtfs.getStations().at(ptr_b->getStationId()) << endl;

    unsigned int sommet = 1;

    while (sommet < chemin.size() - 1)
    {
        ptr_a = ptr_b;
        ++sommet;
        ptr_b = m_arretDuSommet.at(chemin[sommet]);
        while (ptr_b->getStationId() == ptr_a->getStationId())
        {
            ptr_a = ptr_b;
            ++sommet;
            ptr_b = m_arretDuSommet.at(chemin[sommet]);
        }
        //on a changé de station
        if (ptr_b->getStationId() == stationIdDestination) //cas où on est arrivé à la destination
        {
            if (sommet != chemin.size() - 1)
                throw logic_error(
                    "ReseauGTFS::afficherItineraire(): incohérence de fin de chemin lors d'un changement de station");
            break;
        }
        if (sommet == chemin.size() - 1)
            throw logic_error("ReseauGTFS::afficherItineraire(): on ne devrait pas être arrivé à destination");
        //on a changé de station mais sommet n'est pas le noeud destination
        string voyage_id_a = ptr_a->getVoyageId();
        string voyage_id_b = ptr_b->getVoyageId();
        if (voyage_id_a != voyage_id_b) //on a changé de station à pieds
        {
            if (p_afficherItineraire)
                cout << "De cette station, rendez-vous à pieds à la station " << p_gtfs.getStations().at(ptr_b->getStationId()) << endl;
        }
        else //on a changé de station avec un voyage
        {
            Heure heure = ptr_a->getHeureArrivee();
            unsigned int ligne_id = p_gtfs.getVoyages().at(voyage_id_a).getLigne();
            string ligne_numero = p_gtfs.getLignes().at(ligne_id).getNumero();
            if (p_afficherItineraire)
                cout << "De cette station, prenez l'autobus numéro " << ligne_numero << " à l'heure " << heure << " "
            << p_gtfs.getVoyages().at(voyage_id_a) << endl;
            //maintenant allons à la dernière station de ce voyage
            ptr_a = ptr_b;
            ++sommet;
            ptr_b = m_arretDuSommet.at(chemin[sommet]);
            while (ptr_b->getVoyageId() == ptr_a->getVoyageId())
            {
                ptr_a = ptr_b;
                ++sommet;
                ptr_b = m_arretDuSommet.at(chemin[sommet]);
            }
            //on a changé de voyage
            if (p_afficherItineraire)
                cout << "et arrêtez-vous à la station " << p_gtfs.getStations().at(ptr_a->getStationId()) << " à l'heure "
            << ptr_a->getHeureArrivee() << endl;
            if (ptr_b->getStationId() == stationIdDestination) //cas où on est arrivé à la destination
            {
                if (sommet != chemin.size() - 1)
                    throw logic_error(
                        "ReseauGTFS::afficherItineraire(): incohérence de fin de chemin lors d'u changement de voyage");
                break;
            }
            if (ptr_a->getStationId() != ptr_b->getStationId()) //alors on s'est rendu à pieds à l'autre station
                if (p_afficherItineraire)
                    cout << "De cette station, rendez-vous à pieds à la station " << p_gtfs.getStations().at(ptr_b->getStationId()) << endl;
            }
        }

        if (p_afficherItineraire)
        {
            cout << "Déplacez-vous à pieds de cette station au point destination" << endl;
            cout << "Heure d'arrivée à la destination: " << p_gtfs.getTempsDebut().add_secondes(tempsDuTrajet) << endl;
        }
        unsigned int h = tempsDuTrajet / 3600;
        unsigned int reste_sec = tempsDuTrajet % 3600;
        unsigned int m = reste_sec / 60;
        unsigned int s = reste_sec % 60;
        if (p_afficherItineraire)
        {
            cout << "Durée du trajet: " << h << " heures, " << m << " minutes, " << s << " secondes" << endl;
        }

    }


